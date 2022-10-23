/*
  +----------------------------------------------------------------------+
  | APC                                                                  |
  +----------------------------------------------------------------------+
  | Copyright (c) 2006-2011 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http:www.php.net/license/3_01.txt                                    |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Authors: Daniel Cowgill <dcowgill@communityconnect.com>              |
  |          Rasmus Lerdorf <rasmus@php.net>                             |
  |          Arun C. Murthy <arunc@yahoo-inc.com>                        |
  |          Gopal Vijayaraghavan <gopalv@yahoo-inc.com>                 |
  +----------------------------------------------------------------------+

   This software was contributed to PHP by Community Connect Inc. in 2002
   and revised in 2005 by Yahoo! Inc. to add support for PHP 5.1.
   Future revisions and derivatives of this source code must acknowledge
   Community Connect Inc. as the original contributor of this module by
   leaving this note intact in the source code.

   All other licensing and usage conditions are those of the PHP Group.

 */

#include "immutable_cache_cache.h"
#include "immutable_cache_sma.h"
#include "immutable_cache_globals.h"
#include "immutable_cache_strings.h"
#include "immutable_cache_time.h"
#include "php_scandir.h"
#include "SAPI.h"
#include "TSRM.h"
#include "php_main.h"
#include "ext/standard/md5.h"
#include "ext/standard/php_var.h"
#include "zend_smart_str.h"

#if PHP_VERSION_ID < 70300
# define GC_SET_REFCOUNT(ref, rc) (GC_REFCOUNT(ref) = (rc))
# define GC_ADDREF(ref) GC_REFCOUNT(ref)++
#endif

/* If recursive mutexes are used, there is no distinction between read and write locks.
 * As such, if we acquire a read-lock, it's really a write-lock and we are free to perform
 * increments without atomics. */
#ifdef IMMUTABLE_CACHE_LOCK_RECURSIVE
# define ATOMIC_INC_RLOCKED(a) (a)++
#else
# define ATOMIC_INC_RLOCKED(a) ATOMIC_INC(a)
#endif

/* Defined in immutable_cache_persist.c */
immutable_cache_cache_entry_t *immutable_cache_persist(
		immutable_cache_sma_t *sma, immutable_cache_serializer_t *serializer, const immutable_cache_cache_entry_t *orig_entry);
zend_bool immutable_cache_unpersist(zval *dst, const zval *value, immutable_cache_serializer_t *serializer);

/* TODO can this be bigger? */

/* {{{ make_prime */
static int const primes[] = {
  257, /*   256 */
  521, /*   512 */
 1031, /*  1024 */
 2053, /*  2048 */
 3079, /*  3072 */
 4099, /*  4096 */
 5147, /*  5120 */
 6151, /*  6144 */
 7177, /*  7168 */
 8209, /*  8192 */
 9221, /*  9216 */
10243, /* 10240 */
11273, /* 11264 */
12289, /* 12288 */
13313, /* 13312 */
14341, /* 14336 */
15361, /* 15360 */
16411, /* 16384 */
17417, /* 17408 */
18433, /* 18432 */
19457, /* 19456 */
20483, /* 20480 */
30727, /* 30720 */
40961, /* 40960 */
61441, /* 61440 */
81929, /* 81920 */
122887,/* 122880 */
163841,/* 163840 */
245771,/* 245760 */
327689,/* 327680 */
491527,/* 491520 */
655373,/* 655360 */
983063,/* 983040 */
0      /* sentinel */
};

static int make_prime(int n)
{
	int *k = (int*)primes;
	while(*k) {
		if((*k) > n) return *k;
		k++;
	}
	return *(k-1);
}
/* }}} */

static inline void free_entry(immutable_cache_cache_t *cache, immutable_cache_cache_entry_t *entry) {
	immutable_cache_sma_free(cache->sma, entry);
}

/* {{{ immutable_cache_cache_hash_slot
 Note: These calculations can and should be done outside of a lock */
static inline void immutable_cache_cache_hash_slot(
		immutable_cache_cache_t* cache, zend_string *key, zend_ulong* hash, size_t* slot) {
	*hash = ZSTR_HASH(key);
	*slot = *hash % cache->nslots;
} /* }}} */

// TODO will they be the same pointer?
static inline zend_bool immutable_cache_entry_key_equals(const immutable_cache_cache_entry_t *entry, zend_string *key, zend_ulong hash) {
	return ZSTR_H(entry->key) == hash
		&& ZSTR_LEN(entry->key) == ZSTR_LEN(key)
		&& memcmp(ZSTR_VAL(entry->key), ZSTR_VAL(key), ZSTR_LEN(key)) == 0;
}

/* {{{ php serializer */
PHP_IMMUTABLE_CACHE_API int IMMUTABLE_CACHE_SERIALIZER_NAME(php) (IMMUTABLE_CACHE_SERIALIZER_ARGS)
{
	smart_str strbuf = {0};
	php_serialize_data_t var_hash;

	/* Lock in case immutable_cache is accessed inside Serializer::serialize() */
	BG(serialize_lock)++;
	PHP_VAR_SERIALIZE_INIT(var_hash);
	php_var_serialize(&strbuf, (zval*) value, &var_hash);
	PHP_VAR_SERIALIZE_DESTROY(var_hash);
	BG(serialize_lock)--;

	if (EG(exception)) {
		smart_str_free(&strbuf);
		strbuf.s = NULL;
	}

	if (strbuf.s != NULL) {
		*buf = (unsigned char *)estrndup(ZSTR_VAL(strbuf.s), ZSTR_LEN(strbuf.s));
		if (*buf == NULL)
			return 0;

		*buf_len = ZSTR_LEN(strbuf.s);
		smart_str_free(&strbuf);
		return 1;
	}
	return 0;
} /* }}} */

/* {{{ php unserializer */
PHP_IMMUTABLE_CACHE_API int IMMUTABLE_CACHE_UNSERIALIZER_NAME(php) (IMMUTABLE_CACHE_UNSERIALIZER_ARGS)
{
	const unsigned char *tmp = buf;
	php_unserialize_data_t var_hash;
	int result;

	/* Lock in case immutable_cache is accessed inside Serializer::unserialize() */
	BG(serialize_lock)++;
	PHP_VAR_UNSERIALIZE_INIT(var_hash);
	result = php_var_unserialize(value, &tmp, buf + buf_len, &var_hash);
	PHP_VAR_UNSERIALIZE_DESTROY(var_hash);
	BG(serialize_lock)--;

	if (!result) {
		php_error_docref(NULL, E_NOTICE, "Error at offset %ld of %ld bytes", (zend_long)(tmp - buf), (zend_long)buf_len);
		ZVAL_NULL(value);
		return 0;
	}
	return 1;
} /* }}} */

/* {{{ immutable_cache_cache_create */
PHP_IMMUTABLE_CACHE_API immutable_cache_cache_t* immutable_cache_cache_create(immutable_cache_sma_t* sma, immutable_cache_serializer_t* serializer, zend_long size_hint) {
	immutable_cache_cache_t* cache;
	zend_long cache_size;
	size_t nslots;

	/* calculate number of slots */
	nslots = make_prime(size_hint > 0 ? size_hint : 2000);

	/* allocate pointer by normal means */
	cache = pemalloc(sizeof(immutable_cache_cache_t), 1);

	/* calculate cache size for shm allocation */
	cache_size = sizeof(immutable_cache_cache_header_t) + nslots*sizeof(immutable_cache_cache_entry_t *);

	/* allocate shm */
	cache->shmaddr = immutable_cache_sma_malloc(sma, cache_size);

	if (!cache->shmaddr) {
		zend_error_noreturn(E_CORE_ERROR, "Unable to allocate %zu bytes of shared memory for cache structures. Either apc.shm_size is too small or apc.entries_hint too large", cache_size);
		return NULL;
	}

	/* zero cache header and hash slots */
	memset(cache->shmaddr, 0, cache_size);

	/* set default header */
	cache->header = (immutable_cache_cache_header_t*) cache->shmaddr;

	cache->header->nhits = 0;
	cache->header->nmisses = 0;
	cache->header->nentries = 0;
	cache->header->gc = NULL;
	cache->header->stime = time(NULL);
	cache->header->state = 0;

	/* set cache options */
	cache->slots = (immutable_cache_cache_entry_t **) (((char*) cache->shmaddr) + sizeof(immutable_cache_cache_header_t));
	cache->sma = sma;
	cache->serializer = serializer;
	cache->nslots = nslots;

	/* header lock */
	CREATE_LOCK(&cache->header->lock);

	return cache;
} /* }}} */

static inline zend_bool immutable_cache_cache_wlocked_insert_exclusive(
		immutable_cache_cache_t *cache, immutable_cache_cache_entry_t *new_entry) {
	zend_string *key = new_entry->key;

	/* make the insertion */
	{
		immutable_cache_cache_entry_t **entry;
		zend_ulong h;
		size_t s;

		/* calculate hash and entry */
		immutable_cache_cache_hash_slot(cache, key, &h, &s);

		entry = &cache->slots[s];
		while (*entry) {
			/* check for a match by hash and temporary string */
			if (immutable_cache_entry_key_equals(*entry, key, h)) {
                return 0;
			}

			/* set next entry */
			entry = &(*entry)->next;
		}

		/* link in new entry */
		new_entry->next = *entry;
		*entry = new_entry;

		cache->header->mem_size += new_entry->mem_size;
		cache->header->nentries++;
		cache->header->ninserts++;
	}

	return 1;
}

static void immutable_cache_cache_init_entry(
		immutable_cache_cache_entry_t *entry, zend_string *key, const zval* val, time_t t);

static inline zend_bool immutable_cache_cache_store_internal(
		immutable_cache_cache_t *cache, zend_string *key, const zval *val) {
	immutable_cache_cache_entry_t tmp_entry, *entry;
	time_t t = immutable_cache_time();

	/* initialize the entry for insertion */
	immutable_cache_cache_init_entry(&tmp_entry, key, val, t);
	entry = immutable_cache_persist(cache->sma, cache->serializer, &tmp_entry);
	if (!entry) {
		return 0;
	}

	/* execute an insertion */
	if (!immutable_cache_cache_wlocked_insert_exclusive(cache, entry)) {
		free_entry(cache, entry);
		return 0;
	}

	return 1;
}

/* Find entry, without updating stat counters or access time */
static inline immutable_cache_cache_entry_t *immutable_cache_cache_rlocked_find_nostat(
		immutable_cache_cache_t *cache, zend_string *key, time_t t) {
	immutable_cache_cache_entry_t *entry;
	zend_ulong h;
	size_t s;

	/* calculate hash and slot */
	immutable_cache_cache_hash_slot(cache, key, &h, &s);

	entry = cache->slots[s];
	while (entry) {
		/* check for a matching key by has and identifier */
		if (immutable_cache_entry_key_equals(entry, key, h)) {
			return entry;
		}

		entry = entry->next;
	}

	return NULL;
}

/* Find entry, updating stat counters and access time */
static inline immutable_cache_cache_entry_t *immutable_cache_cache_rlocked_find(
		immutable_cache_cache_t *cache, zend_string *key, time_t t) {
	immutable_cache_cache_entry_t *entry;
	zend_ulong h;
	size_t s;

	/* calculate hash and slot */
	immutable_cache_cache_hash_slot(cache, key, &h, &s);

	entry = cache->slots[s];
	while (entry) {
		/* check for a matching key by has and identifier */
		if (immutable_cache_entry_key_equals(entry, key, h)) {
			ATOMIC_INC_RLOCKED(cache->header->nhits);
			ATOMIC_INC_RLOCKED(entry->nhits);
			entry->atime = t;

			return entry;
		}

		entry = entry->next;
	}

	ATOMIC_INC_RLOCKED(cache->header->nmisses);
	return NULL;
}

static inline immutable_cache_cache_entry_t *immutable_cache_cache_rlocked_find_incref(
		immutable_cache_cache_t *cache, zend_string *key, time_t t) {
	immutable_cache_cache_entry_t *entry = immutable_cache_cache_rlocked_find(cache, key, t);
	if (!entry) {
		return NULL;
	}

	return entry;
}

/* {{{ immutable_cache_cache_store */
PHP_IMMUTABLE_CACHE_API zend_bool immutable_cache_cache_store(
		immutable_cache_cache_t* cache, zend_string *key, const zval *val) {
	immutable_cache_cache_entry_t tmp_entry, *entry;
	time_t t = immutable_cache_time();
	zend_bool ret = 0;

	if (!cache) {
		return 0;
	}

	/* initialize the entry for insertion */
	immutable_cache_cache_init_entry(&tmp_entry, key, val, t);
	entry = immutable_cache_persist(cache->sma, cache->serializer, &tmp_entry);
	if (!entry) {
		return 0;
	}

	/* execute an insertion */
	if (!immutable_cache_cache_wlock(cache)) {
		free_entry(cache, entry);
		return 0;
	}

	php_immutable_cache_try {
		ret = immutable_cache_cache_wlocked_insert_exclusive(cache, entry);
	} php_immutable_cache_finally {
		immutable_cache_cache_wunlock(cache);
	} php_immutable_cache_end_try();

	if (!ret) {
		free_entry(cache, entry);
	}

	return ret;
} /* }}} */

#ifndef ZTS
/* {{{ data_unserialize */
static zval data_unserialize(const char *filename)
{
	zval retval;
	zend_long len = 0;
	zend_stat_t sb;
	char *contents, *tmp;
	FILE *fp;
	php_unserialize_data_t var_hash = {0,};

	if(VCWD_STAT(filename, &sb) == -1) {
		return EG(uninitialized_zval);
	}

	fp = fopen(filename, "rb");

	len = sizeof(char)*sb.st_size;

	tmp = contents = malloc(len);

	if(!contents) {
		fclose(fp);
		return EG(uninitialized_zval);
	}

	if(fread(contents, 1, len, fp) < 1) {
		fclose(fp);
		free(contents);
		return EG(uninitialized_zval);
	}

	ZVAL_UNDEF(&retval);

	PHP_VAR_UNSERIALIZE_INIT(var_hash);

	/* I wish I could use json */
	if(!php_var_unserialize(&retval, (const unsigned char**)&tmp, (const unsigned char*)(contents+len), &var_hash)) {
		fclose(fp);
		free(contents);
		return EG(uninitialized_zval);
	}

	PHP_VAR_UNSERIALIZE_DESTROY(var_hash);

	free(contents);
	fclose(fp);

	return retval;
}

static int immutable_cache_load_data(immutable_cache_cache_t* cache, const char *data_file)
{
	char *p;
	char key[MAXPATHLEN] = {0,};
	size_t key_len;
	zval data;

	p = strrchr(data_file, DEFAULT_SLASH);

	if(p && p[1]) {
		strlcpy(key, p+1, sizeof(key));
		p = strrchr(key, '.');

		if(p) {
			p[0] = '\0';
			key_len = strlen(key);

			data = data_unserialize(data_file);
			if(Z_TYPE(data) != IS_UNDEF) {
				zend_string *name = zend_string_init(key, key_len, 0);
				immutable_cache_cache_store(cache, name, &data);
				zend_string_release(name);
				zval_dtor(&data);
			}
			return 1;
		}
	}

	return 0;
}
#endif

/* {{{ immutable_cache_cache_preload shall load the prepared data files in path into the specified cache */
PHP_IMMUTABLE_CACHE_API zend_bool immutable_cache_cache_preload(immutable_cache_cache_t* cache, const char *path)
{
#ifndef ZTS
	zend_bool result = 0;
	char file[MAXPATHLEN]={0,};
	int ndir, i;
	char *p = NULL;
	struct dirent **namelist = NULL;

	if ((ndir = php_scandir(path, &namelist, 0, php_alphasort)) > 0) {
		for (i = 0; i < ndir; i++) {
			/* check for extension */
			if (!(p = strrchr(namelist[i]->d_name, '.'))
					|| (p && strcmp(p, ".data"))) {
				free(namelist[i]);
				continue;
			}

			snprintf(file, MAXPATHLEN, "%s%c%s",
					path, DEFAULT_SLASH, namelist[i]->d_name);

			if(immutable_cache_load_data(cache, file)) {
				result = 1;
			}
			free(namelist[i]);
		}
		free(namelist);
	}
	return result;
#else
	immutable_cache_error("Cannot load data from apc.preload_path=%s in thread-safe mode", path);
	return 0;
#endif
} /* }}} */

/* {{{ immutable_cache_cache_detach */
PHP_IMMUTABLE_CACHE_API void immutable_cache_cache_detach(immutable_cache_cache_t *cache)
{
	/* Important: This function should not clean up anything that's in shared memory,
	 * only detach our process-local use of it. In particular locks cannot be destroyed
	 * here. */

	if (!cache) {
		return;
	}

	free(cache);
}
/* }}} */

/* {{{ immutable_cache_cache_find */
PHP_IMMUTABLE_CACHE_API immutable_cache_cache_entry_t *immutable_cache_cache_find(immutable_cache_cache_t* cache, zend_string *key, time_t t)
{
	immutable_cache_cache_entry_t *entry;

	if (!cache) {
		return NULL;
	}

	if (!immutable_cache_cache_rlock(cache)) {
		return NULL;
	}

	entry = immutable_cache_cache_rlocked_find_incref(cache, key, t);
	immutable_cache_cache_runlock(cache);

	return entry;
}
/* }}} */

/* {{{ immutable_cache_cache_fetch */
PHP_IMMUTABLE_CACHE_API zend_bool immutable_cache_cache_fetch(immutable_cache_cache_t* cache, zend_string *key, time_t t, zval *dst)
{
	immutable_cache_cache_entry_t *entry;
	zend_bool retval = 0;

	if (!cache) {
		return 0;
	}

	if (!immutable_cache_cache_rlock(cache)) {
		return 0;
	}

	entry = immutable_cache_cache_rlocked_find_incref(cache, key, t);
	immutable_cache_cache_runlock(cache);

	if (!entry) {
		return 0;
	}

	retval = immutable_cache_cache_entry_fetch_zval(cache, entry, dst);

	return retval;
} /* }}} */

/* {{{ immutable_cache_cache_exists */
PHP_IMMUTABLE_CACHE_API zend_bool immutable_cache_cache_exists(immutable_cache_cache_t* cache, zend_string *key, time_t t)
{
	immutable_cache_cache_entry_t *entry;

	if (!cache) {
		return 0;
	}

	if (!immutable_cache_cache_rlock(cache)) {
		return 0;
	}

	entry = immutable_cache_cache_rlocked_find_nostat(cache, key, t);
	immutable_cache_cache_runlock(cache);

	return entry != NULL;
}
/* }}} */

/* {{{ immutable_cache_cache_entry_fetch_zval */
PHP_IMMUTABLE_CACHE_API zend_bool immutable_cache_cache_entry_fetch_zval(
		immutable_cache_cache_t *cache, immutable_cache_cache_entry_t *entry, zval *dst)
{
	return immutable_cache_unpersist(dst, &entry->val, cache->serializer);
}
/* }}} */

/* {{{ immutable_cache_cache_make_entry */
static void immutable_cache_cache_init_entry(
		immutable_cache_cache_entry_t *entry, zend_string *key, const zval *val, time_t t)
{
	entry->key = key;
	ZVAL_COPY_VALUE(&entry->val, val);

	entry->next = NULL;
	entry->mem_size = 0;
	entry->nhits = 0;
	entry->ctime = t;
	entry->atime = t;
}
/* }}} */

static inline void array_add_long(zval *array, zend_string *key, zend_long lval) {
	zval zv;
	ZVAL_LONG(&zv, lval);
	zend_hash_add_new(Z_ARRVAL_P(array), key, &zv);
}

static inline void array_add_double(zval *array, zend_string *key, double dval) {
	zval zv;
	ZVAL_DOUBLE(&zv, dval);
	zend_hash_add_new(Z_ARRVAL_P(array), key, &zv);
}

/* {{{ immutable_cache_cache_link_info */
static zval immutable_cache_cache_link_info(immutable_cache_cache_t *cache, immutable_cache_cache_entry_t *p)
{
	zval link, zv;
	array_init(&link);

	ZVAL_STR(&zv, zend_string_dup(p->key, 0));
	zend_hash_add_new(Z_ARRVAL(link), immutable_cache_str_info, &zv);

	array_add_double(&link, immutable_cache_str_num_hits, (double) p->nhits);
	array_add_long(&link, immutable_cache_str_creation_time, p->ctime);
	array_add_long(&link, immutable_cache_str_access_time, p->atime);
	array_add_long(&link, immutable_cache_str_mem_size, p->mem_size);

	return link;
}
/* }}} */

/* {{{ immutable_cache_cache_info */
PHP_IMMUTABLE_CACHE_API zend_bool immutable_cache_cache_info(zval *info, immutable_cache_cache_t *cache, zend_bool limited)
{
	zval list;
	zval gc;
	zval slots;
	immutable_cache_cache_entry_t *p;
	zend_ulong j;

	ZVAL_NULL(info);
	if (!cache) {
		return 0;
	}

	if (!immutable_cache_cache_rlock(cache)) {
		return 0;
	}

	php_immutable_cache_try {
		array_init(info);
		add_assoc_long(info, "num_slots", cache->nslots);
		array_add_long(info, immutable_cache_str_ttl, cache->ttl);
		array_add_double(info, immutable_cache_str_num_hits, (double) cache->header->nhits);
		add_assoc_double(info, "num_misses", (double) cache->header->nmisses);
		add_assoc_double(info, "num_inserts", (double) cache->header->ninserts);
		add_assoc_long(info,   "num_entries", cache->header->nentries);
		add_assoc_long(info, "start_time", cache->header->stime);
		array_add_double(info, immutable_cache_str_mem_size, (double) cache->header->mem_size);

#if IMMUTABLE_CACHE_MMAP
		add_assoc_stringl(info, "memory_type", "mmap", sizeof("mmap")-1);
#else
		add_assoc_stringl(info, "memory_type", "IPC shared", sizeof("IPC shared")-1);
#endif

		if (!limited) {
			size_t i;

			/* For each hashtable slot */
			array_init(&list);
			array_init(&slots);

			for (i = 0; i < cache->nslots; i++) {
				p = cache->slots[i];
				j = 0;
				for (; p != NULL; p = p->next) {
					zval link = immutable_cache_cache_link_info(cache, p);
					add_next_index_zval(&list, &link);
					j++;
				}
				if (j != 0) {
					add_index_long(&slots, (zend_ulong)i, j);
				}
			}

			/* For each slot pending deletion */
			array_init(&gc);

			for (p = cache->header->gc; p != NULL; p = p->next) {
				zval link = immutable_cache_cache_link_info(cache, p);
				add_next_index_zval(&gc, &link);
			}

			add_assoc_zval(info, "cache_list", &list);
			add_assoc_zval(info, "deleted_list", &gc);
			add_assoc_zval(info, "slot_distribution", &slots);
		}
	} php_immutable_cache_finally {
		immutable_cache_cache_runlock(cache);
	} php_immutable_cache_end_try();

	return 1;
}
/* }}} */

/*
 fetches information about the key provided
*/
PHP_IMMUTABLE_CACHE_API void immutable_cache_cache_stat(immutable_cache_cache_t *cache, zend_string *key, zval *stat) {
	zend_ulong h;
	size_t s;

	ZVAL_NULL(stat);
	if (!cache) {
		return;
	}

	/* calculate hash and slot */
	immutable_cache_cache_hash_slot(cache, key, &h, &s);

	if (!immutable_cache_cache_rlock(cache)) {
		return;
	}

	php_immutable_cache_try {
		/* find head */
		immutable_cache_cache_entry_t *entry = cache->slots[s];

		while (entry) {
			/* check for a matching key by has and identifier */
			if (immutable_cache_entry_key_equals(entry, key, h)) {
				array_init(stat);
				array_add_long(stat, immutable_cache_str_hits, entry->nhits);
				array_add_long(stat, immutable_cache_str_access_time, entry->atime);
				array_add_long(stat, immutable_cache_str_creation_time, entry->ctime);
				break;
			}

			/* next */
			entry = entry->next;
		}
	} php_immutable_cache_finally {
		immutable_cache_cache_runlock(cache);
	} php_immutable_cache_end_try();
}

/* {{{ immutable_cache_cache_serializer */
PHP_IMMUTABLE_CACHE_API void immutable_cache_cache_serializer(immutable_cache_cache_t* cache, const char* name) {
	if (cache && !cache->serializer) {
		cache->serializer = immutable_cache_find_serializer(name);
	}
} /* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim>600: noexpandtab sw=4 ts=4 sts=4 fdm=marker
 * vim<600: noexpandtab sw=4 ts=4 sts=4
 */

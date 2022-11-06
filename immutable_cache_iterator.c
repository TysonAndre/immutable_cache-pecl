/*
  +----------------------------------------------------------------------+
  | immutable_cache                                                      |
  +----------------------------------------------------------------------+
  | Copyright (c) 2022 Tyson Andre                                       |
  | This is a fork of the APCu module providing fast immutable caching   |
  | functionality. The original APCu license is below.                   |
  +----------------------------------------------------------------------+
  | Authors of immutable_cache patches: Tyson Andre <tandre@php.net>     |
  +----------------------------------------------------------------------+

  +----------------------------------------------------------------------+
  | APC                                                                  |
  +----------------------------------------------------------------------+
  | Copyright (c) 2006-2011 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Authors: Brian Shire <shire@php.net>                                 |
  +----------------------------------------------------------------------+

 */

#include "php_immutable_cache.h"
#include "immutable_cache_iterator.h"
#include "immutable_cache_cache.h"
#include "immutable_cache_strings.h"
#include "immutable_cache_time.h"
#if PHP_VERSION_ID >= 80000
# include "immutable_cache_iterator_arginfo.h"
#else
# include "immutable_cache_iterator_legacy_arginfo.h"
#endif

#include "ext/standard/md5.h"
#include "SAPI.h"
#include "zend_interfaces.h"

static zend_class_entry *immutable_cache_iterator_ce;
zend_object_handlers immutable_cache_iterator_object_handlers;

zend_class_entry* immutable_cache_iterator_get_ce(void) {
	return immutable_cache_iterator_ce;
}

#define ENSURE_INITIALIZED(iterator) \
	if (!(iterator)->initialized) { \
		zend_throw_error(NULL, "Trying to use uninitialized ImmutableCacheIterator"); \
		return; \
	}

/* {{{ immutable_cache_iterator_item */
static immutable_cache_iterator_item_t* immutable_cache_iterator_item_ctor(
		immutable_cache_iterator_t *iterator, immutable_cache_cache_entry_t *entry) {
	zval zv;
	HashTable *ht;
	immutable_cache_iterator_item_t *item = ecalloc(1, sizeof(immutable_cache_iterator_item_t));

	array_init(&item->value);
	ht = Z_ARRVAL(item->value);

	item->key = zend_string_dup(entry->key, 0);

	if (IMMUTABLE_CACHE_ITER_TYPE & iterator->format) {
		ZVAL_STR_COPY(&zv, immutable_cache_str_user);
		zend_hash_add_new(ht, immutable_cache_str_type, &zv);
	}

	if (IMMUTABLE_CACHE_ITER_KEY & iterator->format) {
		ZVAL_STR_COPY(&zv, item->key);
		zend_hash_add_new(ht, immutable_cache_str_key, &zv);
	}

	if (IMMUTABLE_CACHE_ITER_VALUE & iterator->format) {
		ZVAL_UNDEF(&zv);
		immutable_cache_cache_entry_fetch_zval(immutable_cache_user_cache, entry, &zv);
		zend_hash_add_new(ht, immutable_cache_str_value, &zv);
	}

	if (IMMUTABLE_CACHE_ITER_NUM_HITS & iterator->format) {
		ZVAL_LONG(&zv, entry->nhits);
		zend_hash_add_new(ht, immutable_cache_str_num_hits, &zv);
	}
	if (IMMUTABLE_CACHE_ITER_CTIME & iterator->format) {
		ZVAL_LONG(&zv, entry->ctime);
		zend_hash_add_new(ht, immutable_cache_str_creation_time, &zv);
	}
	if (IMMUTABLE_CACHE_ITER_ATIME & iterator->format) {
		ZVAL_LONG(&zv, entry->atime);
		zend_hash_add_new(ht, immutable_cache_str_access_time, &zv);
	}
	if (IMMUTABLE_CACHE_ITER_MEM_SIZE & iterator->format) {
		ZVAL_LONG(&zv, entry->mem_size);
		zend_hash_add_new(ht, immutable_cache_str_mem_size, &zv);
	}

	return item;
}
/* }}} */

/* {{{ immutable_cache_iterator_item_dtor */
static void immutable_cache_iterator_item_dtor(immutable_cache_iterator_item_t *item) {
	zend_string_release(item->key);
	zval_ptr_dtor(&item->value);
	efree(item);
}
/* }}} */

/* {{{ acp_iterator_free */
static void immutable_cache_iterator_free(zend_object *object) {
	immutable_cache_iterator_t *iterator = immutable_cache_iterator_fetch_from(object);

	if (iterator->initialized == 0) {
		zend_object_std_dtor(object);
		return;
	}

	while (immutable_cache_stack_size(iterator->stack) > 0) {
		immutable_cache_iterator_item_dtor(immutable_cache_stack_pop(iterator->stack));
	}

	immutable_cache_stack_destroy(iterator->stack);

	if (iterator->regex) {
		zend_string_release(iterator->regex);
#if PHP_VERSION_ID >= 70300
		pcre2_match_data_free(iterator->re_match_data);
#endif
	}

	if (iterator->search_hash) {
		zend_hash_destroy(iterator->search_hash);
		efree(iterator->search_hash);
	}
	iterator->initialized = 0;

	zend_object_std_dtor(object);
}
/* }}} */

/* {{{ immutable_cache_iterator_create */
zend_object* immutable_cache_iterator_create(zend_class_entry *ce) {
	immutable_cache_iterator_t *iterator =
		(immutable_cache_iterator_t*) emalloc(sizeof(immutable_cache_iterator_t) + zend_object_properties_size(ce));

	zend_object_std_init(&iterator->obj, ce);
	object_properties_init(&iterator->obj, ce);

	iterator->initialized = 0;
	iterator->stack = NULL;
	iterator->regex = NULL;
	iterator->search_hash = NULL;
	iterator->obj.handlers = &immutable_cache_iterator_object_handlers;

	return &iterator->obj;
}
/* }}} */

/* {{{ immutable_cache_iterator_search_match
 *       Verify if the key matches our search parameters
 */
static int immutable_cache_iterator_search_match(immutable_cache_iterator_t *iterator, immutable_cache_cache_entry_t *entry) {
	int rval = 1;

	if (iterator->regex) {
#if PHP_VERSION_ID >= 70300
		rval = pcre2_match(
			php_pcre_pce_re(iterator->pce),
			(PCRE2_SPTR) ZSTR_VAL(entry->key), ZSTR_LEN(entry->key),
			0, 0, iterator->re_match_data, php_pcre_mctx()) >= 0;
#else
		rval = pcre_exec(
			iterator->pce->re, iterator->pce->extra,
			ZSTR_VAL(entry->key), ZSTR_LEN(entry->key),
			0, 0, NULL, 0) >= 0;
#endif
	}

	if (iterator->search_hash) {
		rval = zend_hash_exists(iterator->search_hash, entry->key);
	}

	return rval;
}
/* }}} */

/* {{{ immutable_cache_iterator_fetch_active */
static size_t immutable_cache_iterator_fetch_active(immutable_cache_iterator_t *iterator) {
	size_t count = 0;
	immutable_cache_iterator_item_t *item;

	while (immutable_cache_stack_size(iterator->stack) > 0) {
		immutable_cache_iterator_item_dtor(immutable_cache_stack_pop(iterator->stack));
	}

	if (!immutable_cache_cache_rlock(immutable_cache_user_cache)) {
		return count;
	}

	php_immutable_cache_try {
		while (count <= iterator->chunk_size && iterator->slot_idx < immutable_cache_user_cache->nslots) {
			immutable_cache_cache_entry_t *entry = immutable_cache_user_cache->slots[iterator->slot_idx];
			while (entry) {
				if (immutable_cache_iterator_search_match(iterator, entry)) {
					count++;
					item = immutable_cache_iterator_item_ctor(iterator, entry);
					if (item) {
						immutable_cache_stack_push(iterator->stack, item);
					}
				}
				entry = entry->next;
			}
			iterator->slot_idx++;
		}
	} php_immutable_cache_finally {
		iterator->stack_idx = 0;
		immutable_cache_cache_runlock(immutable_cache_user_cache);
	} php_immutable_cache_end_try();

	return count;
}
/* }}} */

/* {{{ immutable_cache_iterator_totals */
static void immutable_cache_iterator_totals(immutable_cache_iterator_t *iterator) {
	if (!immutable_cache_cache_rlock(immutable_cache_user_cache)) {
		return;
	}

	php_immutable_cache_try {
		size_t i;

		for (i=0; i < immutable_cache_user_cache->nslots; i++) {
			immutable_cache_cache_entry_t *entry = immutable_cache_user_cache->slots[i];
			while (entry) {
				if (immutable_cache_iterator_search_match(iterator, entry)) {
					iterator->size += entry->mem_size;
					iterator->hits += entry->nhits;
					iterator->count++;
				}
				entry = entry->next;
			}
		}
	} php_immutable_cache_finally {
		iterator->totals_flag = 1;
		immutable_cache_cache_runlock(immutable_cache_user_cache);
	} php_immutable_cache_end_try();
}
/* }}} */

void immutable_cache_iterator_obj_init(immutable_cache_iterator_t *iterator, zval *search, zend_long format, size_t chunk_size, zend_long list)
{
	if (!IMMUTABLE_CACHE_G(enabled)) {
		zend_throw_error(NULL, "ImmutableCache must be enabled to use ImmutableCacheIterator");
		return;
	}

	if (format > IMMUTABLE_CACHE_ITER_ALL) {
		immutable_cache_error("ImmutableCacheIterator format is invalid");
		return;
	}

	if (list == IMMUTABLE_CACHE_LIST_ACTIVE) {
		iterator->fetch = immutable_cache_iterator_fetch_active;
	} else {
		immutable_cache_warning("ImmutableCacheIterator invalid list type");
		return;
	}

	iterator->slot_idx = 0;
	iterator->stack_idx = 0;
	iterator->key_idx = 0;
	iterator->chunk_size = chunk_size == 0 ? IMMUTABLE_CACHE_DEFAULT_CHUNK_SIZE : chunk_size;
	iterator->stack = immutable_cache_stack_create(chunk_size);
	iterator->format = format;
	iterator->totals_flag = 0;
	iterator->count = 0;
	iterator->size = 0;
	iterator->hits = 0;
	iterator->regex = NULL;
	iterator->search_hash = NULL;
	if (search && Z_TYPE_P(search) == IS_STRING && Z_STRLEN_P(search)) {
		iterator->regex = zend_string_copy(Z_STR_P(search));
		iterator->pce = pcre_get_compiled_regex_cache(iterator->regex);

		if (!iterator->pce) {
			immutable_cache_error("Could not compile regular expression: %s", Z_STRVAL_P(search));
			zend_string_release(iterator->regex);
			iterator->regex = NULL;
		}

#if PHP_VERSION_ID >= 70300
		iterator->re_match_data = pcre2_match_data_create_from_pattern(
			php_pcre_pce_re(iterator->pce), php_pcre_gctx());
#endif
	} else if (search && Z_TYPE_P(search) == IS_ARRAY) {
		iterator->search_hash = immutable_cache_flip_hash(Z_ARRVAL_P(search));
	}
	iterator->initialized = 1;
}

PHP_METHOD(ImmutableCacheIterator, __construct) {
	immutable_cache_iterator_t *iterator = immutable_cache_iterator_fetch(getThis());
	zend_long format = IMMUTABLE_CACHE_ITER_ALL;
	zend_long chunk_size = 0;
	zval *search = NULL;
	zend_long list = IMMUTABLE_CACHE_LIST_ACTIVE;

	ZEND_PARSE_PARAMETERS_START(0, 4)
		Z_PARAM_OPTIONAL
		Z_PARAM_ZVAL_EX(search, 1, 0)
		Z_PARAM_LONG(format)
		Z_PARAM_LONG(chunk_size)
		Z_PARAM_LONG(list)
	ZEND_PARSE_PARAMETERS_END();

	if (chunk_size < 0) {
		immutable_cache_error("ImmutableCacheIterator chunk size must be 0 or greater");
		return;
	}

	immutable_cache_iterator_obj_init(iterator, search, format, chunk_size, list);
}

PHP_METHOD(ImmutableCacheIterator, rewind) {
	immutable_cache_iterator_t *iterator = immutable_cache_iterator_fetch(getThis());

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	ENSURE_INITIALIZED(iterator);

	iterator->slot_idx = 0;
	iterator->stack_idx = 0;
	iterator->key_idx = 0;
	iterator->fetch(iterator);
}

PHP_METHOD(ImmutableCacheIterator, valid) {
	immutable_cache_iterator_t *iterator = immutable_cache_iterator_fetch(getThis());

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	ENSURE_INITIALIZED(iterator);

	if (immutable_cache_stack_size(iterator->stack) == iterator->stack_idx) {
		iterator->fetch(iterator);
	}

	RETURN_BOOL(immutable_cache_stack_size(iterator->stack) == 0 ? 0 : 1);
}

PHP_METHOD(ImmutableCacheIterator, current) {
	immutable_cache_iterator_item_t *item;
	immutable_cache_iterator_t *iterator = immutable_cache_iterator_fetch(getThis());

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	ENSURE_INITIALIZED(iterator);

	if (immutable_cache_stack_size(iterator->stack) == iterator->stack_idx) {
		if (iterator->fetch(iterator) == 0) {
			zend_throw_error(NULL, "Cannot call current() on invalid iterator");
			return;
		}
	}

	item = immutable_cache_stack_get(iterator->stack, iterator->stack_idx);
	ZVAL_COPY(return_value, &item->value);
}

PHP_METHOD(ImmutableCacheIterator, key) {
	immutable_cache_iterator_item_t *item;
	immutable_cache_iterator_t *iterator = immutable_cache_iterator_fetch(getThis());

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	ENSURE_INITIALIZED(iterator);
	if (immutable_cache_stack_size(iterator->stack) == iterator->stack_idx) {
		if (iterator->fetch(iterator) == 0) {
			zend_throw_error(NULL, "Cannot call key() on invalid iterator");
			return;
		}
	}

	item = immutable_cache_stack_get(iterator->stack, iterator->stack_idx);

	if (item->key) {
		RETURN_STR_COPY(item->key);
	} else {
		RETURN_LONG(iterator->key_idx);
	}
}

PHP_METHOD(ImmutableCacheIterator, next) {
	immutable_cache_iterator_t *iterator = immutable_cache_iterator_fetch(getThis());

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	ENSURE_INITIALIZED(iterator);
	if (immutable_cache_stack_size(iterator->stack) == 0) {
		return;
	}

	iterator->stack_idx++;
	iterator->key_idx++;
}

PHP_METHOD(ImmutableCacheIterator, getTotalHits) {
	immutable_cache_iterator_t *iterator = immutable_cache_iterator_fetch(getThis());

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	ENSURE_INITIALIZED(iterator);

	if (iterator->totals_flag == 0) {
		immutable_cache_iterator_totals(iterator);
	}

	RETURN_LONG(iterator->hits);
}
/* }}} */

PHP_METHOD(ImmutableCacheIterator, getTotalSize) {
	immutable_cache_iterator_t *iterator = immutable_cache_iterator_fetch(getThis());

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	ENSURE_INITIALIZED(iterator);

	if (iterator->totals_flag == 0) {
		immutable_cache_iterator_totals(iterator);
	}

	RETURN_LONG(iterator->size);
}

PHP_METHOD(ImmutableCacheIterator, getTotalCount) {
	immutable_cache_iterator_t *iterator = immutable_cache_iterator_fetch(getThis());

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	ENSURE_INITIALIZED(iterator);

	if (iterator->totals_flag == 0) {
		immutable_cache_iterator_totals(iterator);
	}

	RETURN_LONG(iterator->count);
}

/* {{{ immutable_cache_iterator_init */
int immutable_cache_iterator_init(int module_number) {
	zend_class_entry ce;

	INIT_CLASS_ENTRY(ce, "ImmutableCacheIterator", class_ImmutableCacheIterator_methods);
	immutable_cache_iterator_ce = zend_register_internal_class(&ce);
	immutable_cache_iterator_ce->create_object = immutable_cache_iterator_create;
#ifndef ZEND_ACC_NO_DYNAMIC_PROPERTIES
#define ZEND_ACC_NO_DYNAMIC_PROPERTIES 0
#endif
	immutable_cache_iterator_ce->ce_flags |= ZEND_ACC_FINAL | ZEND_ACC_NO_DYNAMIC_PROPERTIES;
	zend_class_implements(immutable_cache_iterator_ce, 1, zend_ce_iterator);

	REGISTER_LONG_CONSTANT("IMMUTABLE_CACHE_LIST_ACTIVE", IMMUTABLE_CACHE_LIST_ACTIVE, CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("IMMUTABLE_CACHE_LIST_DELETED", IMMUTABLE_CACHE_LIST_DELETED, CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("IMMUTABLE_CACHE_ITER_TYPE", IMMUTABLE_CACHE_ITER_TYPE, CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("IMMUTABLE_CACHE_ITER_KEY", IMMUTABLE_CACHE_ITER_KEY, CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("IMMUTABLE_CACHE_ITER_VALUE", IMMUTABLE_CACHE_ITER_VALUE, CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("IMMUTABLE_CACHE_ITER_NUM_HITS", IMMUTABLE_CACHE_ITER_NUM_HITS, CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("IMMUTABLE_CACHE_ITER_CTIME", IMMUTABLE_CACHE_ITER_CTIME, CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("IMMUTABLE_CACHE_ITER_ATIME", IMMUTABLE_CACHE_ITER_ATIME, CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("IMMUTABLE_CACHE_ITER_REFCOUNT", IMMUTABLE_CACHE_ITER_REFCOUNT, CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("IMMUTABLE_CACHE_ITER_MEM_SIZE", IMMUTABLE_CACHE_ITER_MEM_SIZE, CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("IMMUTABLE_CACHE_ITER_NONE", IMMUTABLE_CACHE_ITER_NONE, CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("IMMUTABLE_CACHE_ITER_ALL", IMMUTABLE_CACHE_ITER_ALL, CONST_PERSISTENT | CONST_CS);

	memcpy(&immutable_cache_iterator_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

	immutable_cache_iterator_object_handlers.clone_obj = NULL;
	immutable_cache_iterator_object_handlers.free_obj = immutable_cache_iterator_free;
	immutable_cache_iterator_object_handlers.offset = XtOffsetOf(immutable_cache_iterator_t, obj);

	return SUCCESS;
}
/* }}} */

int immutable_cache_iterator_shutdown(int module_number) {
	return SUCCESS;
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim>600: noexpandtab sw=4 ts=4 sts=4 fdm=marker
 * vim<600: noexpandtab sw=4 ts=4 sts=4
 */

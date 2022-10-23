/*
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
  | Authors: Daniel Cowgill <dcowgill@communityconnect.com>              |
  |          Rasmus Lerdorf <rasmus@php.net>                             |
  +----------------------------------------------------------------------+

   This software was contributed to PHP by Community Connect Inc. in 2002
   and revised in 2005 by Yahoo! Inc. to add support for PHP 5.1.
   Future revisions and derivatives of this source code must acknowledge
   Community Connect Inc. as the original contributor of this module by
   leaving this note intact in the source code.

   All other licensing and usage conditions are those of the PHP Group.

 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "immutable_cache_cache.h"
#include "immutable_cache_iterator.h"
#include "immutable_cache_sma.h"
#include "immutable_cache_lock.h"
#include "immutable_cache_mutex.h"
#include "immutable_cache_strings.h"
#include "immutable_cache_time.h"
#include "php_globals.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "ext/standard/file.h"
#include "ext/standard/flock_compat.h"
#include "ext/standard/md5.h"
#include "ext/standard/php_var.h"
#if PHP_VERSION_ID >= 80000
# include "php_immutable_cache_arginfo.h"
#else
# include "php_immutable_cache_legacy_arginfo.h"
#endif
#include "php74_shim.h"

#ifdef HAVE_SYS_FILE_H
#include <sys/file.h>
#endif

#include "SAPI.h"
#include "php_immutable_cache.h"
#include "Zend/zend_smart_str.h"

#if HAVE_SIGACTION
#include "immutable_cache_signal.h"
#endif

/* {{{ ZEND_DECLARE_MODULE_GLOBALS(immutable_cache) */
ZEND_DECLARE_MODULE_GLOBALS(immutable_cache)

/* True globals */
immutable_cache_cache_t* immutable_cache_user_cache = NULL;

/* External APC SMA */
immutable_cache_sma_t immutable_cache_sma;

#define X(str) zend_string *immutable_cache_str_ ## str;
	IMMUTABLE_CACHE_STRINGS
#undef X

/* Global init functions */
static void php_immutable_cache_init_globals(zend_immutable_cache_globals* immutable_cache_globals)
{
	immutable_cache_globals->initialized = 0;
	immutable_cache_globals->smart = 0;
	immutable_cache_globals->preload_path = NULL;
	immutable_cache_globals->coredump_unmap = 0;
	immutable_cache_globals->serializer_name = NULL;
	immutable_cache_globals->entry_level = 0;
}
/* }}} */

/* {{{ PHP_INI */

static PHP_INI_MH(OnUpdateShmSegments) /* {{{ */
{
	zend_long shm_segments = ZEND_STRTOL(new_value->val, NULL, 10);
#if IMMUTABLE_CACHE_MMAP
	if (shm_segments != 1) {
		php_error_docref(NULL, E_WARNING, "apc.shm_segments setting ignored in MMAP mode");
	}
	APCG(shm_segments) = 1;
#else
	APCG(shm_segments) = shm_segments;
#endif
	return SUCCESS;
}
/* }}} */

static PHP_INI_MH(OnUpdateShmSize) /* {{{ */
{
#if PHP_VERSION_ID >= 80200
	zend_long s = zend_ini_parse_quantity_warn(new_value, entry->name);
#else
	zend_long s = zend_atol(new_value->val, new_value->len);
#endif

	if (s <= 0) {
		return FAILURE;
	}

	if (s < Z_L(1048576)) {
		/* if it's less than 1Mb, they are probably using the old syntax */
		php_error_docref(
			NULL, E_WARNING, "apc.shm_size now uses M/G suffixes, please update your ini files");
		s = s * Z_L(1048576);
	}

	APCG(shm_size) = s;

	return SUCCESS;
}
/* }}} */

PHP_INI_BEGIN()
STD_PHP_INI_BOOLEAN("apc.enabled",      "1",    PHP_INI_SYSTEM, OnUpdateBool,              enabled,          zend_immutable_cache_globals, immutable_cache_globals)
STD_PHP_INI_ENTRY("apc.shm_segments",   "1",    PHP_INI_SYSTEM, OnUpdateShmSegments,       shm_segments,     zend_immutable_cache_globals, immutable_cache_globals)
STD_PHP_INI_ENTRY("apc.shm_size",       "32M",  PHP_INI_SYSTEM, OnUpdateShmSize,           shm_size,         zend_immutable_cache_globals, immutable_cache_globals)
STD_PHP_INI_ENTRY("apc.entries_hint",   "4096", PHP_INI_SYSTEM, OnUpdateLong,              entries_hint,     zend_immutable_cache_globals, immutable_cache_globals)
STD_PHP_INI_ENTRY("apc.gc_ttl",         "3600", PHP_INI_SYSTEM, OnUpdateLong,              gc_ttl,           zend_immutable_cache_globals, immutable_cache_globals)
STD_PHP_INI_ENTRY("apc.ttl",            "0",    PHP_INI_SYSTEM, OnUpdateLong,              ttl,              zend_immutable_cache_globals, immutable_cache_globals)
STD_PHP_INI_ENTRY("apc.smart",          "0",    PHP_INI_SYSTEM, OnUpdateLong,              smart,            zend_immutable_cache_globals, immutable_cache_globals)
#if IMMUTABLE_CACHE_MMAP
STD_PHP_INI_ENTRY("apc.mmap_file_mask",  NULL,  PHP_INI_SYSTEM, OnUpdateString,            mmap_file_mask,   zend_immutable_cache_globals, immutable_cache_globals)
#endif
STD_PHP_INI_BOOLEAN("apc.enable_cli",   "0",    PHP_INI_SYSTEM, OnUpdateBool,              enable_cli,       zend_immutable_cache_globals, immutable_cache_globals)
STD_PHP_INI_ENTRY("apc.preload_path", (char*)NULL,              PHP_INI_SYSTEM, OnUpdateString,       preload_path,  zend_immutable_cache_globals, immutable_cache_globals)
STD_PHP_INI_BOOLEAN("apc.coredump_unmap", "0", PHP_INI_SYSTEM, OnUpdateBool, coredump_unmap, zend_immutable_cache_globals, immutable_cache_globals)
STD_PHP_INI_ENTRY("apc.serializer", "php", PHP_INI_SYSTEM, OnUpdateStringUnempty, serializer_name, zend_immutable_cache_globals, immutable_cache_globals)
PHP_INI_END()

/* }}} */

zend_bool immutable_cache_is_enabled(void)
{
	return APCG(enabled);
}

/* {{{ PHP_MINFO_FUNCTION(immutable_cache) */
static PHP_MINFO_FUNCTION(immutable_cache)
{
	php_info_print_table_start();
	php_info_print_table_row(2, "APCu Support", APCG(enabled) ? "Enabled" : "Disabled");
	php_info_print_table_row(2, "Version", PHP_APCU_VERSION);
#ifdef IMMUTABLE_CACHE_DEBUG
	php_info_print_table_row(2, "APCu Debugging", "Enabled");
#else
	php_info_print_table_row(2, "APCu Debugging", "Disabled");
#endif
#if IMMUTABLE_CACHE_MMAP
	php_info_print_table_row(2, "MMAP Support", "Enabled");
	php_info_print_table_row(2, "MMAP File Mask", APCG(mmap_file_mask));
#else
	php_info_print_table_row(2, "MMAP Support", "Disabled");
#endif

	if (APCG(enabled)) {
		immutable_cache_serializer_t *serializer = NULL;
		smart_str names = {0,};
		int i;

		for( i = 0, serializer = immutable_cache_get_serializers();
					serializer->name != NULL;
					serializer++, i++) {
			if (i != 0) {
				smart_str_appends(&names, ", ");
			}
			smart_str_appends(&names, serializer->name);
		}

		if (names.s) {
			smart_str_0(&names);
			php_info_print_table_row(2, "Serialization Support", names.s->val);
			smart_str_free(&names);
		} else {
			php_info_print_table_row(2, "Serialization Support", "Broken");
		}
	} else {
		php_info_print_table_row(2, "Serialization Support", "Disabled");
	}

	php_info_print_table_row(2, "Build Date", __DATE__ " " __TIME__);
	php_info_print_table_end();
	DISPLAY_INI_ENTRIES();
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION(immutable_cache) */
static PHP_MINIT_FUNCTION(immutable_cache)
{
#if defined(ZTS) && defined(COMPILE_DL_APCU)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif
	ZEND_INIT_MODULE_GLOBALS(immutable_cache, php_immutable_cache_init_globals, NULL);

	REGISTER_INI_ENTRIES();

#define X(str) \
	immutable_cache_str_ ## str = zend_new_interned_string( \
		zend_string_init(#str, sizeof(#str) - 1, 1));
	IMMUTABLE_CACHE_STRINGS
#undef X

	/* locks initialized regardless of settings */
	immutable_cache_lock_init();
	IMMUTABLE_CACHE_MUTEX_INIT();

	/* Disable APC in cli mode unless overridden by apc.enable_cli */
	if (!APCG(enable_cli) && !strcmp(sapi_module.name, "cli")) {
		APCG(enabled) = 0;
	}

	/* only run initialization if APC is enabled */
	if (APCG(enabled)) {

		if (!APCG(initialized)) {
#if IMMUTABLE_CACHE_MMAP
			char *mmap_file_mask = APCG(mmap_file_mask);
#else
			char *mmap_file_mask = NULL;
#endif

			/* ensure this runs only once */
			APCG(initialized) = 1;

			/* initialize shared memory allocator */
			immutable_cache_sma_init(
				&immutable_cache_sma,
				APCG(shm_segments), APCG(shm_size), mmap_file_mask);

			REGISTER_LONG_CONSTANT(IMMUTABLE_CACHE_SERIALIZER_CONSTANT, (zend_long)&_apc_register_serializer, CONST_PERSISTENT | CONST_CS);

			/* register default serializer */
			_apc_register_serializer(
				"php", IMMUTABLE_CACHE_SERIALIZER_NAME(php), IMMUTABLE_CACHE_UNSERIALIZER_NAME(php), NULL);

			/* test out the constant function pointer */
			assert(immutable_cache_get_serializers()->name != NULL);

			/* create user cache */
			immutable_cache_user_cache = immutable_cache_cache_create(
				&immutable_cache_sma,
				immutable_cache_find_serializer(APCG(serializer_name)),
				APCG(entries_hint));

			/* preload data from path specified in configuration */
			if (APCG(preload_path)) {
				immutable_cache_cache_preload(
					immutable_cache_user_cache, APCG(preload_path));
			}
		}
	}

	/* initialize iterator object */
	immutable_cache_iterator_init(module_number);

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION(immutable_cache) */
static PHP_MSHUTDOWN_FUNCTION(immutable_cache)
{
#define X(str) zend_string_release(immutable_cache_str_ ## str);
	IMMUTABLE_CACHE_STRINGS
#undef X

	/* locks shutdown regardless of settings */
	immutable_cache_lock_cleanup();
	IMMUTABLE_CACHE_MUTEX_CLEANUP();

	/* only shut down if APC is enabled */
	if (APCG(enabled)) {
		if (APCG(initialized)) {
			/* Detach cache and shared memory allocator from shared memory. */
			immutable_cache_cache_detach(immutable_cache_user_cache);
			immutable_cache_sma_detach(&immutable_cache_sma);

			APCG(initialized) = 0;
		}

#if HAVE_SIGACTION
		immutable_cache_shutdown_signals();
#endif
	}

	immutable_cache_iterator_shutdown(module_number);

	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
} /* }}} */

/* {{{ PHP_RINIT_FUNCTION(immutable_cache) */
static PHP_RINIT_FUNCTION(immutable_cache)
{
#if defined(ZTS) && defined(COMPILE_DL_APCU)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif

	APCG(request_time) = 0;
	if (APCG(enabled)) {
		if (APCG(serializer_name)) {
			/* Avoid race conditions between MINIT of apc and serializer exts like igbinary */
			immutable_cache_cache_serializer(immutable_cache_user_cache, APCG(serializer_name));
		}

#if HAVE_SIGACTION
		immutable_cache_set_signals();
#endif
	}
	return SUCCESS;
}
/* }}} */

/* {{{ proto array immutable_cache_cache_info([bool limited]) */
PHP_FUNCTION(immutable_cache_cache_info)
{
	zend_bool limited = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|b", &limited) == FAILURE) {
		return;
	}

	if (!immutable_cache_cache_info(return_value, immutable_cache_user_cache, limited)) {
		php_error_docref(NULL, E_WARNING, "No APC info available.  Perhaps APC is not enabled? Check apc.enabled in your ini file");
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto array immutable_cache_key_info(string key) */
PHP_FUNCTION(immutable_cache_key_info)
{
	zend_string *key;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &key) == FAILURE) {
		return;
	}

	immutable_cache_cache_stat(immutable_cache_user_cache, key, return_value);
} /* }}} */

/* {{{ proto array immutable_cache_sma_info([bool limited]) */
PHP_FUNCTION(immutable_cache_sma_info)
{
	immutable_cache_sma_info_t* info;
	zval block_lists;
	int i;
	zend_bool limited = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|b", &limited) == FAILURE) {
		return;
	}

	info = immutable_cache_sma_info(&immutable_cache_sma, limited);

	if (!info) {
		php_error_docref(NULL, E_WARNING, "No APC SMA info available.  Perhaps APC is disabled via apc.enabled?");
		RETURN_FALSE;
	}
	array_init(return_value);

	add_assoc_long(return_value, "num_seg", info->num_seg);
	add_assoc_double(return_value, "seg_size", (double)info->seg_size);
	add_assoc_double(return_value, "avail_mem", (double)immutable_cache_sma_get_avail_mem(&immutable_cache_sma));

	if (limited) {
		immutable_cache_sma_free_info(&immutable_cache_sma, info);
		return;
	}

	array_init(&block_lists);

	for (i = 0; i < info->num_seg; i++) {
		immutable_cache_sma_link_t* p;
		zval list;

		array_init(&list);
		for (p = info->list[i]; p != NULL; p = p->next) {
			zval link;

			array_init(&link);

			add_assoc_long(&link, "size", p->size);
			add_assoc_long(&link, "offset", p->offset);
			add_next_index_zval(&list, &link);
		}
		add_next_index_zval(&block_lists, &list);
	}
	add_assoc_zval(return_value, "block_lists", &block_lists);
	immutable_cache_sma_free_info(&immutable_cache_sma, info);
}
/* }}} */

/* {{{ immutable_cache_store_helper(INTERNAL_FUNCTION_PARAMETERS, const zend_bool exclusive)
 */
static void immutable_cache_store_helper(INTERNAL_FUNCTION_PARAMETERS)
{
	zval *key;
	zval *val = NULL;
	zend_long ttl = 0L;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "z|zl", &key, &val, &ttl) == FAILURE) {
		return;
	}

	if (APCG(serializer_name)) {
		/* Avoid race conditions between MINIT of apc and serializer exts like igbinary */
		immutable_cache_cache_serializer(immutable_cache_user_cache, APCG(serializer_name));
	}

	/* TODO: Port to array|string for PHP 8? */
	if (Z_TYPE_P(key) == IS_ARRAY) {
		zval *hentry;
		zend_string *hkey;
		zend_ulong hkey_idx;
		HashTable* hash = Z_ARRVAL_P(key);

		/* We only insert keys that failed */
		zval fail_zv;
		ZVAL_LONG(&fail_zv, -1);
		array_init(return_value);

		ZEND_HASH_FOREACH_KEY_VAL(hash, hkey_idx, hkey, hentry) {
			ZVAL_DEREF(hentry);
			if (hkey) {
				zend_string_addref(hkey);
			} else {
				hkey = zend_long_to_str(hkey_idx);
			}
			if (!immutable_cache_cache_store(immutable_cache_user_cache, hkey, hentry)) {
				zend_symtable_add_new(Z_ARRVAL_P(return_value), hkey, &fail_zv);
			}
			zend_string_release(hkey);
		} ZEND_HASH_FOREACH_END();
		return;
	} else if (Z_TYPE_P(key) == IS_STRING) {
		if (!val) {
			/* nothing to store */
			RETURN_FALSE;
		}

		RETURN_BOOL(immutable_cache_cache_store(immutable_cache_user_cache, Z_STR_P(key), val));
	} else {
		immutable_cache_warning("immutable_cache_store expects key parameter to be a string or an array of key/value pairs.");
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto bool immutable_cache_enabled(void)
	returns true when immutable_cache is usable in the current environment */
PHP_FUNCTION(immutable_cache_enabled) {
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	RETURN_BOOL(APCG(enabled));
}
/* }}} */

/* {{{ proto int immutable_cache_add(mixed key, mixed var [, long ttl ])
 */
PHP_FUNCTION(immutable_cache_add) {
	immutable_cache_store_helper(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto mixed immutable_cache_fetch(mixed key[, bool &success])
 */
PHP_FUNCTION(immutable_cache_fetch) {
	zval *key;
	zval *success = NULL;
	time_t t;
	int result;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "z|z", &key, &success) == FAILURE) {
		return;
	}

	t = immutable_cache_time();

	if (Z_TYPE_P(key) != IS_STRING && Z_TYPE_P(key) != IS_ARRAY) {
		convert_to_string(key);
	}

	/* TODO: Port to array|string for PHP 8? */
	if (Z_TYPE_P(key) == IS_STRING) {
		result = immutable_cache_cache_fetch(immutable_cache_user_cache, Z_STR_P(key), t, return_value);
	} else if (Z_TYPE_P(key) == IS_ARRAY) {
		zval *hentry;

		array_init(return_value);
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(key), hentry) {
			ZVAL_DEREF(hentry);
			if (Z_TYPE_P(hentry) == IS_STRING) {
				zval result_entry;
				ZVAL_UNDEF(&result_entry);

				if (immutable_cache_cache_fetch(immutable_cache_user_cache, Z_STR_P(hentry), t, &result_entry)) {
					zend_hash_update(Z_ARRVAL_P(return_value), Z_STR_P(hentry), &result_entry);
				}
			} else {
				immutable_cache_warning("immutable_cache_fetch() expects a string or array of strings.");
			}
		} ZEND_HASH_FOREACH_END();
		result = 1;
	} else {
		immutable_cache_warning("immutable_cache_fetch() expects a string or array of strings.");
		result = 0;
	}

	if (success) {
		ZEND_TRY_ASSIGN_REF_BOOL(success, result);
	}
	if (!result) {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto mixed immutable_cache_exists(mixed key)
 */
PHP_FUNCTION(immutable_cache_exists) {
	zval *key;
	time_t t;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &key) == FAILURE) {
		return;
	}

	t = immutable_cache_time();

	if (Z_TYPE_P(key) != IS_STRING && Z_TYPE_P(key) != IS_ARRAY) {
		convert_to_string(key);
	}

	/* TODO: Port to array|string for PHP 8? */
	if (Z_TYPE_P(key) == IS_STRING) {
		RETURN_BOOL(immutable_cache_cache_exists(immutable_cache_user_cache, Z_STR_P(key), t));
	} else if (Z_TYPE_P(key) == IS_ARRAY) {
		zval *hentry;
		zval true_zv;
		ZVAL_TRUE(&true_zv);

		array_init(return_value);
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(key), hentry) {
			ZVAL_DEREF(hentry);
			if (Z_TYPE_P(hentry) == IS_STRING) {
				if (immutable_cache_cache_exists(immutable_cache_user_cache, Z_STR_P(hentry), t)) {
					  zend_hash_add_new(Z_ARRVAL_P(return_value), Z_STR_P(hentry), &true_zv);
				}
			} else {
				immutable_cache_warning(
					"immutable_cache_exists() expects a string or array of strings.");
			}
		} ZEND_HASH_FOREACH_END();
	} else {
		immutable_cache_warning("immutable_cache_exists() expects a string or array of strings.");
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ module definition structure */

zend_module_entry immutable_cache_module_entry = {
	STANDARD_MODULE_HEADER,
	PHP_APCU_EXTNAME,
	ext_functions,
	PHP_MINIT(immutable_cache),
	PHP_MSHUTDOWN(immutable_cache),
	PHP_RINIT(immutable_cache),
	NULL,
	PHP_MINFO(immutable_cache),
	PHP_APCU_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_APCU
ZEND_GET_MODULE(immutable_cache)
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE();
#endif
#endif
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim>600: noexpandtab sw=4 ts=4 sts=4 fdm=marker
 * vim<600: noexpandtab sw=4 ts=4 sts=4
 */

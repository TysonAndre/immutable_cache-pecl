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
  | Authors: Daniel Cowgill <dcowgill@communityconnect.com>              |
  |          George Schlossnagle <george@omniti.com>                     |
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

#include "immutable_cache.h"
#include "immutable_cache_cache.h"
#include "immutable_cache_globals.h"
#include "php.h"

/* {{{ console display functions */
#define IMMUTABLE_CACHE_PRINT_FUNCTION(name, verbosity)					\
	void immutable_cache_##name(const char *format, ...)				\
	{									\
		va_list args;							\
										\
		va_start(args, format);						\
		php_verror(NULL, "", verbosity, format, args);			\
		va_end(args);							\
	}

IMMUTABLE_CACHE_PRINT_FUNCTION(error, E_ERROR)
IMMUTABLE_CACHE_PRINT_FUNCTION(warning, E_WARNING)
IMMUTABLE_CACHE_PRINT_FUNCTION(notice, E_NOTICE)

#ifdef IMMUTABLE_CACHE_DEBUG
IMMUTABLE_CACHE_PRINT_FUNCTION(debug, E_NOTICE)
#else
void immutable_cache_debug(const char *format, ...) {}
#endif
/* }}} */

/* {{{ immutable_cache_flip_hash */
HashTable* immutable_cache_flip_hash(HashTable *hash) {
	zval data, *entry;
	HashTable *new_hash;

	if(hash == NULL) return hash;

	ZVAL_LONG(&data, 1);

	ALLOC_HASHTABLE(new_hash);
	zend_hash_init(new_hash, zend_hash_num_elements(hash), NULL, ZVAL_PTR_DTOR, 0);

	ZEND_HASH_FOREACH_VAL(hash, entry) {
		ZVAL_DEREF(entry);
		if (Z_TYPE_P(entry) == IS_STRING) {
			zend_hash_update(new_hash, Z_STR_P(entry), &data);
		} else {
			zend_hash_index_update(new_hash, Z_LVAL_P(entry), &data);
		}
	} ZEND_HASH_FOREACH_END();

	return new_hash;
}
/* }}} */

/*
* Serializer API
*/
#define IMMUTABLE_CACHE_MAX_SERIALIZERS 16

/* pointer to the list of serializers */
static immutable_cache_serializer_t immutable_cache_serializers[IMMUTABLE_CACHE_MAX_SERIALIZERS] = {{0,}};
/* }}} */

/* {{{ immutable_cache_register_serializer */
PHP_IMMUTABLE_CACHE_API int _immutable_cache_register_serializer(
        const char* name, immutable_cache_serialize_t serialize, immutable_cache_unserialize_t unserialize, void *config) {
	int i;
	immutable_cache_serializer_t *serializer;
	if (strcmp(name, "default") == 0) {
		php_error_docref(NULL, E_WARNING, "_immutable_cache_register_serializer: The serializer name 'default' is reserved.");
		return 0;
	}

	for(i = 0; i < IMMUTABLE_CACHE_MAX_SERIALIZERS; i++) {
		serializer = &immutable_cache_serializers[i];
		if(!serializer->name) {
			/* empty entry */
			serializer->name = name;
			serializer->serialize = serialize;
			serializer->unserialize = unserialize;
			serializer->config = config;
			if (i < IMMUTABLE_CACHE_MAX_SERIALIZERS - 1) {
				immutable_cache_serializers[i+1].name = NULL;
			}
			return 1;
		}
	}

	return 0;
} /* }}} */

/* {{{ immutable_cache_get_serializers */
PHP_IMMUTABLE_CACHE_API immutable_cache_serializer_t* immutable_cache_get_serializers()  {
	return &(immutable_cache_serializers[0]);
} /* }}} */

/* {{{ immutable_cache_find_serializer */
PHP_IMMUTABLE_CACHE_API immutable_cache_serializer_t* immutable_cache_find_serializer(const char* name) {
	int i;
	immutable_cache_serializer_t *serializer;

	for(i = 0; i < IMMUTABLE_CACHE_MAX_SERIALIZERS; i++) {
		serializer = &immutable_cache_serializers[i];
		if(serializer->name && (strcmp(serializer->name, name) == 0)) {
			return serializer;
		}
	}
	if (strcmp(name, "default") != 0) {
		zend_string *names = immutable_cache_get_supported_serializer_names();
		php_error_docref(NULL, E_WARNING, "Could not find immutable_cache.serializer='%s'. Supported serializers: %s", name, ZSTR_VAL(names));
		zend_string_release(names);
	}
	return NULL;
} /* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim>600: noexpandtab sw=4 ts=4 sts=4 fdm=marker
 * vim<600: noexpandtab sw=4 ts=4 sts=4
 */

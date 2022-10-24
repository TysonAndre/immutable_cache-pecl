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

#ifndef IMMUTABLE_CACHE_H
#define IMMUTABLE_CACHE_H

#include "php_version.h"

#if PHP_VERSION_ID < 80000
/* Not tested yet, this will crash in older php versions */
#error This release of immutable_cache only supports php 8.0+
#endif

/*
 * This module defines utilities and helper functions used elsewhere in APC.
 */
#ifdef PHP_WIN32
# define PHP_IMMUTABLE_CACHE_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
# define PHP_IMMUTABLE_CACHE_API __attribute__ ((visibility("default")))
#else
# define PHP_IMMUTABLE_CACHE_API
#endif

/* Commonly needed C library headers. */
#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* UNIX headers (needed for struct stat) */
#include <sys/types.h>
#include <sys/stat.h>
#ifndef PHP_WIN32
#include <unistd.h>
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "php.h"
#include "main/php_streams.h"

/* console display functions */
PHP_IMMUTABLE_CACHE_API void immutable_cache_error(const char *format, ...) ZEND_ATTRIBUTE_FORMAT(printf, 1, 2);
PHP_IMMUTABLE_CACHE_API void immutable_cache_warning(const char *format, ...) ZEND_ATTRIBUTE_FORMAT(printf, 1, 2);
PHP_IMMUTABLE_CACHE_API void immutable_cache_notice(const char *format, ...) ZEND_ATTRIBUTE_FORMAT(printf, 1, 2);
PHP_IMMUTABLE_CACHE_API void immutable_cache_debug(const char *format, ...) ZEND_ATTRIBUTE_FORMAT(printf, 1, 2);

/* immutable_cache_flip_hash flips keys and values for faster searching */
PHP_IMMUTABLE_CACHE_API HashTable* immutable_cache_flip_hash(HashTable *hash);

#if defined(__GNUC__)
# define IMMUTABLE_CACHE_UNUSED __attribute__((unused))
# define IMMUTABLE_CACHE_USED __attribute__((used))
# define IMMUTABLE_CACHE_ALLOC __attribute__((malloc))
# if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__  > 2)
#  define IMMUTABLE_CACHE_HOTSPOT __attribute__((hot))
# else
#  define IMMUTABLE_CACHE_HOTSPOT
# endif
#else
# define IMMUTABLE_CACHE_UNUSED
# define IMMUTABLE_CACHE_USED
# define IMMUTABLE_CACHE_ALLOC
# define IMMUTABLE_CACHE_HOTSPOT
#endif

/*
* Serializer API
*/
#define IMMUTABLE_CACHE_SERIALIZER_ABI "0"
#define IMMUTABLE_CACHE_SERIALIZER_CONSTANT "\000immutable_cache_register_serializer-" IMMUTABLE_CACHE_SERIALIZER_ABI

#define IMMUTABLE_CACHE_SERIALIZER_NAME(module) module##_immutable_cache_serializer
#define IMMUTABLE_CACHE_UNSERIALIZER_NAME(module) module##_immutable_cache_unserializer

#define IMMUTABLE_CACHE_SERIALIZER_ARGS unsigned char **buf, size_t *buf_len, const zval *value, void *config
#define IMMUTABLE_CACHE_UNSERIALIZER_ARGS zval *value, unsigned char *buf, size_t buf_len, void *config

typedef int (*immutable_cache_serialize_t)(IMMUTABLE_CACHE_SERIALIZER_ARGS);
typedef int (*immutable_cache_unserialize_t)(IMMUTABLE_CACHE_UNSERIALIZER_ARGS);

/* {{{ struct definition: immutable_cache_serializer_t */
typedef struct immutable_cache_serializer_t {
	const char*        name;
	immutable_cache_serialize_t    serialize;
	immutable_cache_unserialize_t  unserialize;
	void*              config;
} immutable_cache_serializer_t;
/* }}} */

/* {{{ _immutable_cache_register_serializer
 registers the serializer using the given name and parameters */
PHP_IMMUTABLE_CACHE_API int _immutable_cache_register_serializer(
        const char* name, immutable_cache_serialize_t serialize, immutable_cache_unserialize_t unserialize, void *config);
/* }}} */

/* {{{ immutable_cache_get_serializers
 fetches the list of serializers */
PHP_IMMUTABLE_CACHE_API immutable_cache_serializer_t* immutable_cache_get_serializers(void); /* }}} */

/* {{{ immutable_cache_find_serializer
 finds a previously registered serializer by name */
PHP_IMMUTABLE_CACHE_API immutable_cache_serializer_t* immutable_cache_find_serializer(const char* name); /* }}} */

/* {{{ default serializers */
PHP_IMMUTABLE_CACHE_API int IMMUTABLE_CACHE_SERIALIZER_NAME(php) (IMMUTABLE_CACHE_SERIALIZER_ARGS);
PHP_IMMUTABLE_CACHE_API int IMMUTABLE_CACHE_UNSERIALIZER_NAME(php) (IMMUTABLE_CACHE_UNSERIALIZER_ARGS); /* }}} */

/* {{{ eval serializers */
PHP_IMMUTABLE_CACHE_API int IMMUTABLE_CACHE_SERIALIZER_NAME(eval) (IMMUTABLE_CACHE_SERIALIZER_ARGS);
PHP_IMMUTABLE_CACHE_API int IMMUTABLE_CACHE_UNSERIALIZER_NAME(eval) (IMMUTABLE_CACHE_UNSERIALIZER_ARGS); /* }}} */

#define php_immutable_cache_try                        \
{                                          \
	JMP_BUF *zb = EG(bailout);             \
	JMP_BUF ab;                            \
	zend_bool _bailout = 0;                \
	                                       \
	EG(bailout) = &ab;                     \
	if (SETJMP(ab) == SUCCESS) {

#define php_immutable_cache_finally                    \
	} else {                               \
		_bailout = 1;                      \
	}

#define php_immutable_cache_end_try()                  \
	EG(bailout) = zb;                      \
	if (_bailout) {                        \
		zend_bailout();                    \
	}                                      \
}

#define php_immutable_cache_try_finish() (EG(bailout) = zb)

#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim>600: noexpandtab sw=4 ts=4 sts=4 fdm=marker
 * vim<600: noexpandtab sw=4 ts=4 sts=4
 */

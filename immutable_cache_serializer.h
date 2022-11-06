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
  | http://www.php.net/license/3_01.txt.                                 |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Authors: Gopal Vijayaraghavan <gopalv@php.net>                       |
  +----------------------------------------------------------------------+

 */

#ifndef IMMUTABLE_CACHE_SERIALIZER_H
#define IMMUTABLE_CACHE_SERIALIZER_H

/* this is a shipped .h file, do not include any other header in this file */
#define IMMUTABLE_CACHE_SERIALIZER_NAME(module) module##_immmutable_cache_serializer
#define IMMUTABLE_CACHE_UNSERIALIZER_NAME(module) module##_immmutable_cache_unserializer

#define IMMUTABLE_CACHE_SERIALIZER_ARGS unsigned char **buf, size_t *buf_len, const zval *value, void *config
#define IMMUTABLE_CACHE_UNSERIALIZER_ARGS zval *value, unsigned char *buf, size_t buf_len, void *config

typedef int (*immutable_cache_serialize_t)(IMMUTABLE_CACHE_SERIALIZER_ARGS);
typedef int (*immutable_cache_unserialize_t)(IMMUTABLE_CACHE_UNSERIALIZER_ARGS);

typedef int (*immutable_cache_register_serializer_t)(const char* name, immutable_cache_serialize_t serialize, immutable_cache_unserialize_t unserialize, void *config);

/*
 * ABI version for constant hooks. Increment this any time you make any changes
 * to any function in this file.
 */
#define IMMUTABLE_CACHE_SERIALIZER_ABI "0"
#define IMMUTABLE_CACHE_SERIALIZER_CONSTANT "\000immutable_cache_register_serializer-" IMMUTABLE_CACHE_SERIALIZER_ABI

#if !defined(IMMUTABLE_CACHE_UNUSED)
# if defined(__GNUC__)
#  define IMMUTABLE_CACHE_UNUSED __attribute__((unused))
# else
# define IMMUTABLE_CACHE_UNUSED
# endif
#endif

static IMMUTABLE_CACHE_UNUSED int immutable_cache_register_serializer(
        const char* name, immutable_cache_serialize_t serialize, immutable_cache_unserialize_t unserialize, void *config)
{
	int retval = 0;

	zend_string *lookup = zend_string_init(
		IMMUTABLE_CACHE_SERIALIZER_CONSTANT, sizeof(IMMUTABLE_CACHE_SERIALIZER_CONSTANT)-1, 0);
	zval *magic = zend_get_constant(lookup);

	/* zend_get_constant will return 1 on success, otherwise immutable_cache_magic_constant wouldn't be touched at all */
	if (magic) {
		immutable_cache_register_serializer_t register_func = (immutable_cache_register_serializer_t)(Z_LVAL_P(magic));
		if(register_func) {
			retval = register_func(name, serialize, unserialize, NULL);
		}
	}

	zend_string_release(lookup);

	return retval;
}

#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim>600: noexpandtab sw=4 ts=4 sts=4 fdm=marker
 * vim<600: noexpandtab sw=4 ts=4 sts=4
 */

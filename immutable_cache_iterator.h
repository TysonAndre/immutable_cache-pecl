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
  | Authors: Brian Shire <shire@.php.net>                                |
  +----------------------------------------------------------------------+

 */

#ifndef IMMUTABLE_CACHE_ITERATOR_H
#define IMMUTABLE_CACHE_ITERATOR_H

#include "immutable_cache.h"
#include "immutable_cache_stack.h"

#include "ext/pcre/php_pcre.h"
#include "zend_smart_str.h"

#define IMMUTABLE_CACHE_DEFAULT_CHUNK_SIZE 100

#define IMMUTABLE_CACHE_LIST_ACTIVE   0x1
#define IMMUTABLE_CACHE_LIST_DELETED  0x2

#define IMMUTABLE_CACHE_ITER_TYPE		(1 << 0)
#define IMMUTABLE_CACHE_ITER_KEY        (1 << 1)
#define IMMUTABLE_CACHE_ITER_VALUE      (1 << 2)
#define IMMUTABLE_CACHE_ITER_NUM_HITS   (1 << 3)
#define IMMUTABLE_CACHE_ITER_CTIME      (1 << 5)
#define IMMUTABLE_CACHE_ITER_ATIME      (1 << 7)
#define IMMUTABLE_CACHE_ITER_REFCOUNT   (1 << 8)
#define IMMUTABLE_CACHE_ITER_MEM_SIZE   (1 << 9)

#define IMMUTABLE_CACHE_ITER_NONE       0
#define IMMUTABLE_CACHE_ITER_ALL        (0xffffffffL)

/* {{{ immutable_cache_iterator_t */
typedef struct _immmutable_cache_iterator_t {
	short int initialized;   /* sanity check in case __construct failed */
	zend_ulong format;             /* format bitmask of the return values ie: key, value, info */
	size_t (*fetch)(struct _immmutable_cache_iterator_t *iterator);
							 /* fetch callback to fetch items from cache slots or lists */
	size_t slot_idx;           /* index to the slot array or linked list */
	size_t chunk_size;         /* number of entries to pull down per fetch */
	immutable_cache_stack_t *stack;      /* stack of entries pulled from cache */
	int stack_idx;           /* index into the current stack */
	pcre_cache_entry *pce;     /* regex filter on entry identifiers */
#if PHP_VERSION_ID >= 70300
	pcre2_match_data *re_match_data; /* match data for regex */
#endif
	zend_string *regex;
	HashTable *search_hash;  /* hash of keys to iterate over */
	zend_long key_idx;            /* incrementing index for numerical keys */
	short int totals_flag;   /* flag if totals have been calculated */
	zend_long hits;               /* hit total */
	size_t size;             /* size total */
	zend_long count;              /* count total */
	zend_object obj;
} immutable_cache_iterator_t;
/* }}} */

#define immutable_cache_iterator_fetch_from(o) ((immutable_cache_iterator_t*)((char*)o - XtOffsetOf(immutable_cache_iterator_t, obj)))
#define immutable_cache_iterator_fetch(z) immutable_cache_iterator_fetch_from(Z_OBJ_P(z))

/* {{{ immutable_cache_iterator_item */
typedef struct _immmutable_cache_iterator_item_t {
	zend_string *key;
	zval value;
} immutable_cache_iterator_item_t;
/* }}} */

PHP_IMMUTABLE_CACHE_API void immutable_cache_iterator_obj_init(
	immutable_cache_iterator_t *iterator,
	zval *search,
	zend_ulong format,
	size_t chunk_size,
	zend_long list);
PHP_IMMUTABLE_CACHE_API zend_class_entry* immutable_cache_iterator_get_ce(void);
PHP_IMMUTABLE_CACHE_API int immutable_cache_iterator_init(int module_number);
PHP_IMMUTABLE_CACHE_API int immutable_cache_iterator_shutdown(int module_number);

#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim>600: noexpandtab sw=4 ts=4 sts=4 fdm=marker
 * vim<600: noexpandtab sw=4 ts=4 sts=4
 */

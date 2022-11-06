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
  +----------------------------------------------------------------------+

   This software was contributed to PHP by Community Connect Inc. in 2002
   and revised in 2005 by Yahoo! Inc. to add support for PHP 5.1.
   Future revisions and derivatives of this source code must acknowledge
   Community Connect Inc. as the original contributor of this module by
   leaving this note intact in the source code.

   All other licensing and usage conditions are those of the PHP Group.

 */

#ifndef IMMUTABLE_CACHE_SMA_H
#define IMMUTABLE_CACHE_SMA_H

/* {{{ SMA API
	APC SMA API provides support for shared memory allocators to external libraries ( and to APC )
	Skip to the bottom macros for error free usage of the SMA API
*/

#include "immutable_cache.h"

/* {{{ struct definition: immutable_cache_segment_t */
typedef struct _immutable_cache_segment_t {
	size_t size;            /* size of this segment */
	void* shmaddr;          /* address of shared memory */
#ifdef IMMUTABLE_CACHE_MEMPROTECT
	void* roaddr;           /* read only (mprotect'd) address */
#endif
} immutable_cache_segment_t; /* }}} */

/* {{{ struct definition: immutable_cache_sma_link_t */
typedef struct immutable_cache_sma_link_t immutable_cache_sma_link_t;
struct immutable_cache_sma_link_t {
	zend_long size;              /* size of this free block */
	zend_long offset;            /* offset in segment of this block */
	immutable_cache_sma_link_t* next;   /* link to next free block */
};
/* }}} */

/* {{{ struct definition: immutable_cache_sma_info_t */
typedef struct immutable_cache_sma_info_t immutable_cache_sma_info_t;
struct immutable_cache_sma_info_t {
	int num_seg;            /* number of segments */
	size_t seg_size;        /* segment size */
	immutable_cache_sma_link_t** list;  /* one list per segment of links */
};
/* }}} */

/* {{{ struct definition: immutable_cache_sma_t */
typedef struct _immutable_cache_sma_t {
	zend_bool initialized;         /* flag to indicate this sma has been initialized */

	/* info */
	int32_t  num;                  /* number of segments */
	size_t size;                   /* segment size */
	int32_t  last;                 /* last segment */

	/* segments */
	immutable_cache_segment_t *segs;           /* segments */
} immutable_cache_sma_t; /* }}} */

/*
* immutable_cache_sma_api_init will initialize a shared memory allocator with num segments of the given size
*
* should be called once per allocator per process
*/
PHP_IMMUTABLE_CACHE_API void immutable_cache_sma_init(
		immutable_cache_sma_t* sma,
		int32_t num, size_t size, char *mask);

/*
 * immutable_cache_sma_detach will detach from shared memory and cleanup local allocations.
 */
PHP_IMMUTABLE_CACHE_API void immutable_cache_sma_detach(immutable_cache_sma_t* sma);

/*
* immutable_cache_smap_api_malloc will allocate a block from the sma of the given size
*/
PHP_IMMUTABLE_CACHE_API void* immutable_cache_sma_malloc(immutable_cache_sma_t* sma, size_t size);

/*
 * immutable_cache_sma_api_malloc_ex will allocate a block from the sma of the given size and
 * provide the size of the actual allocation.
 */
PHP_IMMUTABLE_CACHE_API void *immutable_cache_sma_malloc_ex(
		immutable_cache_sma_t *sma, size_t size, size_t *allocated);

/*
* immutable_cache_sma_api_free will free p (which should be a pointer to a block allocated from sma)
*/
PHP_IMMUTABLE_CACHE_API void immutable_cache_sma_free(immutable_cache_sma_t* sma, void* p);

/*
* immutable_cache_sma_api_protect will protect p (which should be a pointer to a block allocated from sma)
*/
PHP_IMMUTABLE_CACHE_API void* immutable_cache_sma_protect(immutable_cache_sma_t* sma, void* p);

/*
* immutable_cache_sma_api_protect will uprotect p (which should be a pointer to a block allocated from sma)
*/
PHP_IMMUTABLE_CACHE_API void* immutable_cache_sma_unprotect(immutable_cache_sma_t* sma, void *p);

/*
* immutable_cache_sma_api_info returns information about the allocator
*/
PHP_IMMUTABLE_CACHE_API immutable_cache_sma_info_t* immutable_cache_sma_info(immutable_cache_sma_t* sma, zend_bool limited);

/*
* immutable_cache_sma_api_info_free_info is for freeing immutable_cache_sma_info_t* returned by immutable_cache_sma_api_info
*/
PHP_IMMUTABLE_CACHE_API void immutable_cache_sma_free_info(immutable_cache_sma_t* sma, immutable_cache_sma_info_t* info);

/*
* immutable_cache_sma_api_get_avail_mem will return the amount of memory available left to sma
*/
PHP_IMMUTABLE_CACHE_API size_t immutable_cache_sma_get_avail_mem(immutable_cache_sma_t* sma);

/*
* immutable_cache_sma_api_get_avail_size will return true if at least size bytes are available to the sma
*/
PHP_IMMUTABLE_CACHE_API zend_bool immutable_cache_sma_get_avail_size(immutable_cache_sma_t* sma, size_t size);

/*
* immutable_cache_sma_api_check_integrity will check the integrity of sma
*/
PHP_IMMUTABLE_CACHE_API void immutable_cache_sma_check_integrity(immutable_cache_sma_t* sma); /* }}} */

/**
 * Check if ptr is within the shared memory region(s) managed by the immutable cache
 */
PHP_IMMUTABLE_CACHE_API bool immutable_cache_sma_contains_pointer(const immutable_cache_sma_t *sma, const void *ptr);

/* {{{ ALIGNWORD: pad up x, aligned to the system's word boundary */
typedef union { void* p; int i; long l; double d; void (*f)(void); } immutable_cache_word_t;
#define ALIGNSIZE(x, size) ((size) * (1 + (((x)-1)/(size))))
#define ALIGNWORD(x) ALIGNSIZE(x, sizeof(immutable_cache_word_t))
/* }}} */

#endif


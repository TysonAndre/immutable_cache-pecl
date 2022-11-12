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

#ifndef IMMUTABLE_CACHE_CACHE_H
#define IMMUTABLE_CACHE_CACHE_H

#include "immutable_cache.h"
#include "immutable_cache_sma.h"
#include "immutable_cache_lock.h"
#include "immutable_cache_globals.h"
#include "TSRM.h"

typedef struct immutable_cache_cache_slam_key_t immutable_cache_cache_slam_key_t;
struct immutable_cache_cache_slam_key_t {
	zend_ulong hash;         /* hash of the key */
	size_t len;              /* length of the key */
	time_t mtime;            /* creation time of this key */
	pid_t owner_pid;         /* the pid that created this key */
#ifdef ZTS
	void ***owner_thread;    /* TSRMLS cache of thread that created this key */
#endif
};

/* {{{ struct definition: immutable_cache_cache_entry_t */
typedef struct immutable_cache_cache_entry_t immutable_cache_cache_entry_t;
struct immutable_cache_cache_entry_t {
    /* TODO Can this be made interned and const, similar to opcache */
	zend_string *key;        /* entry key */
	zval val;                /* the zval copied at store time */
	immutable_cache_cache_entry_t *next; /* next entry in linked list */
	zend_long nhits;         /* number of hits to this entry */
	time_t ctime;            /* time entry was initialized */
	time_t atime;            /* time entry was last accessed */
	zend_long mem_size;      /* memory used */
};
/* }}} */

/* {{{ struct definition: immutable_cache_cache_header_t
   Any values that must be shared among processes should go in here. */
typedef struct _immmutable_cache_cache_header_t {
	immutable_cache_lock_t lock;                /* header lock */
	zend_long nhits;                /* hit count */
	zend_long nmisses;              /* miss count */
	zend_long ninserts;             /* insert count */
	zend_long nexpunges;            /* expunge count */
	zend_long nentries;             /* entry count */
	zend_long mem_size;             /* used */
	time_t stime;                   /* start time */
	unsigned short state;           /* cache state */
	immutable_cache_cache_slam_key_t lastkey;   /* last key inserted (not necessarily without error) */
	immutable_cache_cache_entry_t *gc;          /* gc list */
} immutable_cache_cache_header_t; /* }}} */

/* {{{ struct definition: immutable_cache_cache_t */
typedef struct _immmutable_cache_cache_t {
	// FIXME when is this process local?
	void* shmaddr;                /* process (local) address of shared cache */
	immutable_cache_cache_header_t* header;   /* cache header (stored in SHM) */
	immutable_cache_cache_entry_t** slots;    /* array of cache slots (stored in SHM) */
	immutable_cache_sma_t* sma;               /* shared memory allocator */
	immutable_cache_serializer_t* serializer; /* serializer */
	size_t nslots;                /* number of slots in cache */
	zend_bool loaded_serializer;
} immutable_cache_cache_t; /* }}} */

/* {{{ typedef: immutable_cache_cache_updater_t */
typedef zend_bool (*immutable_cache_cache_updater_t)(immutable_cache_cache_t*, immutable_cache_cache_entry_t*, void* data); /* }}} */

/* {{{ typedef: immutable_cache_cache_atomic_updater_t */
typedef zend_bool (*immutable_cache_cache_atomic_updater_t)(immutable_cache_cache_t*, zend_long*, void* data); /* }}} */

/*
 * immutable_cache_cache_create creates the shared memory cache.
 *
 * This function should be called once per process per cache
 *
 * serializer for APCu is set by globals on MINIT and ensured with immutable_cache_cache_serializer
 * during execution. Using immutable_cache_cache_serializer avoids race conditions between MINIT/RINIT of
 * APCU and the third party serializer. API users can choose to leave this null to use default
 * PHP serializers, or search the list of serializers for the preferred serializer
 *
 * size_hint is a "hint" at the total number entries that will be expected.
 * It determines the physical size of the hash table. Passing 0 for
 * this argument will use a reasonable default value
 */
PHP_IMMUTABLE_CACHE_API immutable_cache_cache_t* immutable_cache_cache_create(
        immutable_cache_sma_t* sma, immutable_cache_serializer_t* serializer, zend_long size_hint);
/*
* immutable_cache_cache_preload preloads the data at path into the specified cache
*/
PHP_IMMUTABLE_CACHE_API zend_bool immutable_cache_cache_preload(immutable_cache_cache_t* cache, const char* path);

/*
 * immutable_cache_cache_detach detaches from the shared memory cache and cleans up
 * local allocations. Under apache, this function can be safely called by
 * the child processes when they exit.
 */
PHP_IMMUTABLE_CACHE_API void immutable_cache_cache_detach(immutable_cache_cache_t* cache);

/*
 * immutable_cache_cache_clear empties a cache. This can safely be called at any time.
 */
PHP_IMMUTABLE_CACHE_API void immutable_cache_cache_clear(immutable_cache_cache_t* cache);

/*
 * immutable_cache_cache_store creates key, entry and context in which to make an insertion of val into the specified cache
 */
PHP_IMMUTABLE_CACHE_API zend_bool immutable_cache_cache_store(
        immutable_cache_cache_t* cache, zend_string *key, const zval *val);
/*
 * immutable_cache_cache_update updates an entry in place. The updater function must not bailout.
 * The update is performed under write-lock and doesn't have to be atomic.
 */
PHP_IMMUTABLE_CACHE_API zend_bool immutable_cache_cache_update(
		immutable_cache_cache_t *cache, zend_string *key, immutable_cache_cache_updater_t updater, void *data,
		zend_bool insert_if_not_found, zend_long ttl);

/*
 * immutable_cache_cache_find searches for a cache entry by its hashed identifier,
 * and returns a pointer to the entry if found, NULL otherwise.
 *
 */
PHP_IMMUTABLE_CACHE_API immutable_cache_cache_entry_t* immutable_cache_cache_find(immutable_cache_cache_t* cache, zend_string *key, time_t t);

/*
 * immutable_cache_cache_fetch fetches an entry from the cache directly into dst
 *
 */
PHP_IMMUTABLE_CACHE_API zend_bool immutable_cache_cache_fetch(immutable_cache_cache_t* cache, zend_string *key, time_t t, zval *dst);

/*
 * immutable_cache_cache_exists searches for a cache entry by its hashed identifier,
 * and returns whether the entry exists.
 */
PHP_IMMUTABLE_CACHE_API zend_bool immutable_cache_cache_exists(immutable_cache_cache_t* cache, zend_string *key, time_t t);

/* immutable_cache_cache_fetch_zval copies a cache entry value to be usable at runtime.
 */
PHP_IMMUTABLE_CACHE_API zend_bool immutable_cache_cache_entry_fetch_zval(
		immutable_cache_cache_t *cache, immutable_cache_cache_entry_t *entry, zval *dst);

/*
 * immutable_cache_cache_entry_release decrements the reference count associated with a cache
 * entry. Calling immutable_cache_cache_find automatically increments the reference count,
 * and this function must be called post-execution to return the count to its
 * original value. Failing to do so will prevent the entry from being
 * garbage-collected.
 *
 * entry is the cache entry whose ref count you want to decrement.
 */
PHP_IMMUTABLE_CACHE_API void immutable_cache_cache_entry_release(immutable_cache_cache_t *cache, immutable_cache_cache_entry_t *entry);

/*
 fetches information about the cache provided for userland status functions
*/
PHP_IMMUTABLE_CACHE_API zend_bool immutable_cache_cache_info(zval *info, immutable_cache_cache_t *cache, zend_bool limited);

/*
 fetches information about the key provided
*/
PHP_IMMUTABLE_CACHE_API void immutable_cache_cache_stat(immutable_cache_cache_t *cache, zend_string *key, zval *stat);

/*
* immutable_cache_cache_serializer
* sets the serializer for a cache, and by proxy contexts created for the cache
* Note: this avoids race conditions between third party serializers and APCu
*/
PHP_IMMUTABLE_CACHE_API void immutable_cache_cache_serializer(immutable_cache_cache_t* cache, const char* name);

/* immutable_cache_entry() holds a write lock on the cache while executing user code.
 * That code may call other immutable_cache_* functions, which also try to acquire a
 * read or write lock, which would deadlock. As such, don't try to acquire a
 * lock if the current thread is inside immutable_cache_entry().
 *
 * Whether the current thread is inside immutable_cache_entry() is tracked by IMMUTABLE_CACHE_G(entry_level).
 * This breaks the self-contained immutable_cache_cache_t abstraction, but is currently
 * necessary because the entry_level needs to be tracked per-thread, while
 * immutable_cache_cache_t is a per-process structure.
 */

static inline zend_bool immutable_cache_cache_wlock(immutable_cache_cache_t *cache) {
	if (!IMMUTABLE_CACHE_G(entry_level)) {
		return WLOCK(&cache->header->lock);
	}
	return 1;
}

static inline void immutable_cache_cache_wunlock(immutable_cache_cache_t *cache) {
	if (!IMMUTABLE_CACHE_G(entry_level)) {
		WUNLOCK(&cache->header->lock);
	}
}

static inline zend_bool immutable_cache_cache_rlock(immutable_cache_cache_t *cache) {
	if (!IMMUTABLE_CACHE_G(entry_level)) {
		return RLOCK(&cache->header->lock);
	}
	return 1;
}

static inline void immutable_cache_cache_runlock(immutable_cache_cache_t *cache) {
	if (!IMMUTABLE_CACHE_G(entry_level)) {
		RUNLOCK(&cache->header->lock);
	}
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

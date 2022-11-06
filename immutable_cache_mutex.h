/*
  +----------------------------------------------------------------------+
  | APCu                                                                 |
  +----------------------------------------------------------------------+
  | Copyright (c) 2013 The PHP Group                                     |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Joe Watkins <joe.watkins@live.co.uk>                        |
  +----------------------------------------------------------------------+
 */

#ifndef IMMUTABLE_CACHE_MUTEX_H
#define IMMUTABLE_CACHE_MUTEX_H

#include "immutable_cache.h"

#ifdef IMMUTABLE_CACHE_HAS_PTHREAD_MUTEX

#include "pthread.h"

typedef pthread_mutex_t immutable_cache_mutex_t;

PHP_IMMUTABLE_CACHE_API zend_bool immutable_cache_mutex_init(void);
PHP_IMMUTABLE_CACHE_API void immutable_cache_mutex_cleanup(void);
PHP_IMMUTABLE_CACHE_API zend_bool immutable_cache_mutex_create(immutable_cache_mutex_t *lock);
PHP_IMMUTABLE_CACHE_API zend_bool immutable_cache_mutex_lock(immutable_cache_mutex_t *lock);
PHP_IMMUTABLE_CACHE_API zend_bool immutable_cache_mutex_unlock(immutable_cache_mutex_t *lock);
PHP_IMMUTABLE_CACHE_API void immutable_cache_mutex_destroy(immutable_cache_mutex_t *lock);

#define IMMUTABLE_CACHE_MUTEX_INIT()          immutable_cache_mutex_init()
#define IMMUTABLE_CACHE_MUTEX_CLEANUP()       immutable_cache_mutex_cleanup()

#define IMMUTABLE_CACHE_CREATE_MUTEX(lock)    immutable_cache_mutex_create(lock)
#define IMMUTABLE_CACHE_DESTROY_MUTEX(lock)   immutable_cache_mutex_destroy(lock)
#define IMMUTABLE_CACHE_MUTEX_LOCK(lock)      immutable_cache_mutex_lock(lock)
#define IMMUTABLE_CACHE_MUTEX_UNLOCK(lock)    immutable_cache_mutex_unlock(lock)

#else

#include "immutable_cache_lock.h"

typedef immutable_cache_lock_t immutable_cache_mutex_t;

// Fallback to normal locks

#define IMMUTABLE_CACHE_MUTEX_INIT()
#define IMMUTABLE_CACHE_MUTEX_CLEANUP()

#define IMMUTABLE_CACHE_CREATE_MUTEX(lock)    CREATE_LOCK(lock)
#define IMMUTABLE_CACHE_DESTROY_MUTEX(lock)   DESTROY_LOCK(lock)
#define IMMUTABLE_CACHE_MUTEX_LOCK(lock)      WLOCK(lock)
#define IMMUTABLE_CACHE_MUTEX_UNLOCK(lock)    WUNLOCK(lock)

#endif

#endif

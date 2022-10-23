/*
  +----------------------------------------------------------------------+
  | APCu                                                                 |
  +----------------------------------------------------------------------+
  | Copyright (c) 2013 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Fabian Franz <fabian@lionsad.de>                             |
  +----------------------------------------------------------------------+
 */
#include "immutable_cache_mutex.h"

#ifdef IMMUTABLE_CACHE_HAS_PTHREAD_MUTEX

static zend_bool immutable_cache_mutex_ready = 0;
static pthread_mutexattr_t immutable_cache_mutex_attr;

PHP_IMMUTABLE_CACHE_API zend_bool immutable_cache_mutex_init() {
	if (immutable_cache_mutex_ready) {
		return 1;
	}
	immutable_cache_mutex_ready = 1;

	if (pthread_mutexattr_init(&immutable_cache_mutex_attr) != SUCCESS) {
		return 0;
	}

	if (pthread_mutexattr_setpshared(&immutable_cache_mutex_attr, PTHREAD_PROCESS_SHARED) != SUCCESS) {
		return 0;
	}

	return 1;
}

PHP_IMMUTABLE_CACHE_API void immutable_cache_mutex_cleanup() {
	if (!immutable_cache_mutex_ready) {
		return;
	}
	immutable_cache_mutex_ready = 0;

	pthread_mutexattr_destroy(&immutable_cache_mutex_attr);
}

PHP_IMMUTABLE_CACHE_API zend_bool immutable_cache_mutex_create(immutable_cache_mutex_t *lock) {
	pthread_mutex_init(lock, &immutable_cache_mutex_attr);
	return 1;
}

PHP_IMMUTABLE_CACHE_API zend_bool immutable_cache_mutex_lock(immutable_cache_mutex_t *lock) {
	HANDLE_BLOCK_INTERRUPTIONS();
	if (pthread_mutex_lock(lock) == 0) {
		return 1;
	}

	HANDLE_UNBLOCK_INTERRUPTIONS();
	immutable_cache_warning("Failed to acquire lock");
	return 0;
}

PHP_IMMUTABLE_CACHE_API zend_bool immutable_cache_mutex_unlock(immutable_cache_mutex_t *lock) {
	pthread_mutex_unlock(lock);
	HANDLE_UNBLOCK_INTERRUPTIONS();
	return 1;
}

PHP_IMMUTABLE_CACHE_API void immutable_cache_mutex_destroy(immutable_cache_mutex_t *lock) {
	pthread_mutex_destroy(lock);
}

#endif

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
  | Author: Joe Watkins <joe.watkins@live.co.uk>                        |
  +----------------------------------------------------------------------+
 */

#ifndef HAVE_APC_LOCK_H
# include "immutable_cache_lock.h"
#endif

/*
 * While locking calls should never fail, apcu checks for the success of write-lock
 * acquisitions, to prevent more damage when a deadlock is detected.
 */

#ifdef PHP_WIN32
PHP_APCU_API zend_bool immutable_cache_lock_init() {
	return 1;
}

PHP_APCU_API void immutable_cache_lock_cleanup() {
}

PHP_APCU_API zend_bool immutable_cache_lock_create(immutable_cache_lock_t *lock) {
	return NULL != immutable_cache_windows_cs_create(lock);
}

static inline zend_bool immutable_cache_lock_rlock_impl(immutable_cache_lock_t *lock) {
	immutable_cache_windows_cs_rdlock(lock);
	return 1;
}

static inline zend_bool immutable_cache_lock_wlock_impl(immutable_cache_lock_t *lock) {
	immutable_cache_windows_cs_lock(lock);
	return 1;
}

PHP_APCU_API zend_bool immutable_cache_lock_wunlock(immutable_cache_lock_t *lock) {
	immutable_cache_windows_cs_unlock_wr(lock);
	return 1;
}

PHP_APCU_API zend_bool immutable_cache_lock_runlock(immutable_cache_lock_t *lock) {
	immutable_cache_windows_cs_unlock_rd(lock);
	return 1;
}

PHP_APCU_API void immutable_cache_lock_destroy(immutable_cache_lock_t *lock) {
	immutable_cache_windows_cs_destroy(lock);
}

#elif defined(IMMUTABLE_CACHE_NATIVE_RWLOCK)

static zend_bool immutable_cache_lock_ready = 0;
static pthread_rwlockattr_t immutable_cache_lock_attr;

PHP_APCU_API zend_bool immutable_cache_lock_init() {
	if (immutable_cache_lock_ready) {
		return 1;
	}
	immutable_cache_lock_ready = 1;

	if (pthread_rwlockattr_init(&immutable_cache_lock_attr) != SUCCESS) {
		return 0;
	}
	if (pthread_rwlockattr_setpshared(&immutable_cache_lock_attr, PTHREAD_PROCESS_SHARED) != SUCCESS) {
		return 0;
	}
	return 1;
}

PHP_APCU_API void immutable_cache_lock_cleanup() {
	if (!immutable_cache_lock_ready) {
		return;
	}
	immutable_cache_lock_ready = 0;

	pthread_rwlockattr_destroy(&immutable_cache_lock_attr);
}

PHP_APCU_API zend_bool immutable_cache_lock_create(immutable_cache_lock_t *lock) {
	return pthread_rwlock_init(lock, &immutable_cache_lock_attr) == SUCCESS;
}

static inline zend_bool immutable_cache_lock_rlock_impl(immutable_cache_lock_t *lock) {
	return pthread_rwlock_rdlock(lock) == 0;
}

static inline zend_bool immutable_cache_lock_wlock_impl(immutable_cache_lock_t *lock) {
	return pthread_rwlock_wrlock(lock) == 0;
}

PHP_APCU_API zend_bool immutable_cache_lock_wunlock(immutable_cache_lock_t *lock) {
	pthread_rwlock_unlock(lock);
	return 1;
}

PHP_APCU_API zend_bool immutable_cache_lock_runlock(immutable_cache_lock_t *lock) {
	pthread_rwlock_unlock(lock);
	return 1;
}

PHP_APCU_API void immutable_cache_lock_destroy(immutable_cache_lock_t *lock) {
	pthread_rwlock_destroy(lock);
}

#elif defined(IMMUTABLE_CACHE_LOCK_RECURSIVE)

static zend_bool immutable_cache_lock_ready = 0;
static pthread_mutexattr_t immutable_cache_lock_attr;

PHP_APCU_API zend_bool immutable_cache_lock_init() {
	if (immutable_cache_lock_ready) {
		return 1;
	}
	immutable_cache_lock_ready = 1;

	if (pthread_mutexattr_init(&immutable_cache_lock_attr) != SUCCESS) {
		return 0;
	}

	if (pthread_mutexattr_setpshared(&immutable_cache_lock_attr, PTHREAD_PROCESS_SHARED) != SUCCESS) {
		return 0;
	}

	pthread_mutexattr_settype(&immutable_cache_lock_attr, PTHREAD_MUTEX_RECURSIVE);
	return 1;
}

PHP_APCU_API void immutable_cache_lock_cleanup() {
	if (!immutable_cache_lock_ready) {
		return;
	}
	immutable_cache_lock_ready = 0;

	pthread_mutexattr_destroy(&immutable_cache_lock_attr);
}

PHP_APCU_API zend_bool immutable_cache_lock_create(immutable_cache_lock_t *lock) {
	pthread_mutex_init(lock, &immutable_cache_lock_attr);
	return 1;
}

static inline zend_bool immutable_cache_lock_rlock_impl(immutable_cache_lock_t *lock) {
	return pthread_mutex_lock(lock) == 0;
}

static inline zend_bool immutable_cache_lock_wlock_impl(immutable_cache_lock_t *lock) {
	return pthread_mutex_lock(lock) == 0;
}

PHP_APCU_API zend_bool immutable_cache_lock_wunlock(immutable_cache_lock_t *lock) {
	pthread_mutex_unlock(lock);
	return 1;
}

PHP_APCU_API zend_bool immutable_cache_lock_runlock(immutable_cache_lock_t *lock) {
	pthread_mutex_unlock(lock);
	return 1;
}

PHP_APCU_API void immutable_cache_lock_destroy(immutable_cache_lock_t *lock) {
	pthread_mutex_destroy(lock);
}

#elif defined(IMMUTABLE_CACHE_SPIN_LOCK)

static int immutable_cache_lock_try(immutable_cache_lock_t *lock) {
	int failed = 1;

	asm volatile
	(
		"xchgl %0, 0(%1)" :
		"=r" (failed) : "r" (&lock->state),
		"0" (failed)
	);

	return failed;
}

static int immutable_cache_lock_get(immutable_cache_lock_t *lock) {
	int failed = 1;

	do {
		failed = immutable_cache_lock_try(lock);
#ifdef IMMUTABLE_CACHE_LOCK_NICE
		usleep(0);
#endif
	} while (failed);

	return failed;
}

static int immutable_cache_lock_release(immutable_cache_lock_t *lock) {
	int released = 0;

	asm volatile (
		"xchg %0, 0(%1)" : "=r" (released) : "r" (&lock->state),
		"0" (released)
	);

	return !released;
}

PHP_APCU_API zend_bool immutable_cache_lock_init() {
	return 0;
}

PHP_APCU_API void immutable_cache_lock_cleanup() {
}

PHP_APCU_API zend_bool immutable_cache_lock_create(immutable_cache_lock_t *lock) {
	lock->state = 0;
}

static inline zend_bool immutable_cache_lock_rlock_impl(immutable_cache_lock_t *lock) {
	immutable_cache_lock_get(lock);
	return 1;
}

static inline zend_bool immutable_cache_lock_wlock_impl(immutable_cache_lock_t *lock) {
	immutable_cache_lock_get(lock);
	return 1;
}

PHP_APCU_API zend_bool immutable_cache_lock_wunlock(immutable_cache_lock_t *lock) {
	immutable_cache_lock_release(lock);
	return 1;
}

PHP_APCU_API zend_bool immutable_cache_lock_runlock(immutable_cache_lock_t *lock) {
	immutable_cache_lock_release(lock);
	return 1;
}

PHP_APCU_API void immutable_cache_lock_destroy(immutable_cache_lock_t *lock) {
}

#else

#include <unistd.h>
#include <fcntl.h>

static int immutable_cache_fcntl_call(int fd, int cmd, int type, off_t offset, int whence, off_t len) {
	int ret;
	struct flock lock;

	lock.l_type = type;
	lock.l_start = offset;
	lock.l_whence = whence;
	lock.l_len = len;
	lock.l_pid = 0;

	do {
		ret = fcntl(fd, cmd, &lock) ;
	} while(ret < 0 && errno == EINTR);

	return(ret);
}

PHP_APCU_API zend_bool immutable_cache_lock_init() {
	return 0;
}

PHP_APCU_API void immutable_cache_lock_cleanup() {
}

PHP_APCU_API zend_bool immutable_cache_lock_create(immutable_cache_lock_t *lock) {
	char lock_path[] = "/tmp/.apc.XXXXXX";

	*lock = mkstemp(lock_path);
	if (*lock > 0) {
		unlink(lock_path);
		return 1;
	} else {
		return 0;
	}
}

static inline zend_bool immutable_cache_lock_rlock_impl(immutable_cache_lock_t *lock) {
	immutable_cache_fcntl_call((*lock), F_SETLKW, F_RDLCK, 0, SEEK_SET, 0);
	return 1;
}

static inline zend_bool immutable_cache_lock_wlock_impl(immutable_cache_lock_t *lock) {
	immutable_cache_fcntl_call((*lock), F_SETLKW, F_WRLCK, 0, SEEK_SET, 0);
	return 1;
}

PHP_APCU_API zend_bool immutable_cache_lock_wunlock(immutable_cache_lock_t *lock) {
	immutable_cache_fcntl_call((*lock), F_SETLKW, F_UNLCK, 0, SEEK_SET, 0);
	return 1;
}

PHP_APCU_API zend_bool immutable_cache_lock_runlock(immutable_cache_lock_t *lock) {
	immutable_cache_fcntl_call((*lock), F_SETLKW, F_UNLCK, 0, SEEK_SET, 0);
	return 1;
}

PHP_APCU_API void immutable_cache_lock_destroy(immutable_cache_lock_t *lock) {
	close(*lock);
}

#endif

/* Shared for all lock implementations */

PHP_APCU_API zend_bool immutable_cache_lock_wlock(immutable_cache_lock_t *lock) {
	HANDLE_BLOCK_INTERRUPTIONS();
	if (immutable_cache_lock_wlock_impl(lock)) {
		return 1;
	}

	HANDLE_UNBLOCK_INTERRUPTIONS();
	immutable_cache_warning("Failed to acquire write lock");
	return 0;
}

PHP_APCU_API zend_bool immutable_cache_lock_rlock(immutable_cache_lock_t *lock) {
	HANDLE_BLOCK_INTERRUPTIONS();
	if (immutable_cache_lock_rlock_impl(lock)) {
		return 1;
	}

	HANDLE_UNBLOCK_INTERRUPTIONS();
	immutable_cache_warning("Failed to acquire read lock");
	return 0;
}

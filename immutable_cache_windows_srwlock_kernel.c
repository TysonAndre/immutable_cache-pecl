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
  | Authors: Pierre Joye <pierre@php.net>                                |
  +----------------------------------------------------------------------+
 */

#include <php.h>

#ifdef IMMUTABLE_CACHE_SRWLOCK_KERNEL
#include "immutable_cache_windows_srwlock_kernel.h"

immutable_cache_windows_cs_rwlock_t *immutable_cache_windows_cs_create(immutable_cache_windows_cs_rwlock_t *lock)
{
	InitializeSRWLock(lock);

	return lock;
}

void immutable_cache_windows_cs_destroy(immutable_cache_windows_cs_rwlock_t *lock)
{
	/* pass */
	return;
}

void immutable_cache_windows_cs_lock(immutable_cache_windows_cs_rwlock_t *lock)
{
	AcquireSRWLockExclusive(lock);
}

void immutable_cache_windows_cs_rdlock(immutable_cache_windows_cs_rwlock_t *lock)
{
	AcquireSRWLockShared(lock);
}

void immutable_cache_windows_cs_unlock_rd(immutable_cache_windows_cs_rwlock_t *lock)
{
	ReleaseSRWLockShared(lock);
}

void immutable_cache_windows_cs_unlock_wr(immutable_cache_windows_cs_rwlock_t *lock)
{
	ReleaseSRWLockExclusive(lock);
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

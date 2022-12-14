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

#ifndef IMMUTABLE_CACHE_GLOBALS_H
#define IMMUTABLE_CACHE_GLOBALS_H

#include "immutable_cache.h"

ZEND_BEGIN_MODULE_GLOBALS(immutable_cache)
	/* configuration parameters */
	zend_long shm_segments;      /* number of shared memory segments to use */
	zend_long shm_size;          /* size of each shared memory segment (in MB) */
	zend_long entries_hint;      /* hint at the number of entries expected */

#if IMMUTABLE_CACHE_MMAP
	char *mmap_file_mask;   /* mktemp-style file-mask to pass to mmap */
#endif

	/* module variables */
	zend_bool initialized;       /* true if module was initialized */
	zend_bool enable_cli;        /* Flag to override turning immutable_cache off for CLI */
	zend_bool protect_memory;    /* Flag to protect memory with mprotect */
	zend_bool enabled;           /* if true, immutable_cache is enabled (defaults to true) */

	char *preload_path;          /* preload path */
	zend_bool coredump_unmap;    /* trap signals that coredump and unmap shared memory */

	char *serializer_name;       /* the serializer config option */

	/* Nesting level of immutable_cache_entry calls. */
	unsigned int entry_level;
ZEND_END_MODULE_GLOBALS(immutable_cache)

/* (the following is defined in php_immutable_cache.c) */
ZEND_EXTERN_MODULE_GLOBALS(immutable_cache)

#ifdef ZTS
# define IMMUTABLE_CACHE_G(v) TSRMG(immutable_cache_globals_id, zend_immutable_cache_globals *, v)
#else
# define IMMUTABLE_CACHE_G(v) (immutable_cache_globals.v)
#endif

#if defined(HAVE_MPROTECT) || defined(ZEND_WIN32)
#define IMMUTABLE_CACHE_SHOULD_PROTECT_MEMORY() IMMUTABLE_CACHE_G(protect_memory)
#else
#define IMMUTABLE_CACHE_SHOULD_PROTECT_MEMORY() (0)
#endif

extern struct _immmutable_cache_cache_t* immutable_cache_user_cache;

#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim>600: noexpandtab sw=4 ts=4 sts=4 fdm=marker
 * vim<600: noexpandtab sw=4 ts=4 sts=4
 */

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
  |          George Schlossnagle <george@omniti.com>                     |
  |          Rasmus Lerdorf <rasmus@php.net>                             |
  +----------------------------------------------------------------------+

   This software was contributed to PHP by Community Connect Inc. in 2002
   and revised in 2005 by Yahoo! Inc. to add support for PHP 5.1.
   Future revisions and derivatives of this source code must acknowledge
   Community Connect Inc. as the original contributor of this module by
   leaving this note intact in the source code.

   All other licensing and usage conditions are those of the PHP Group.

 */

#ifndef PHP_IMMUTABLE_CACHE_H
#define PHP_IMMUTABLE_CACHE_H

#include "immutable_cache.h"
#include "immutable_cache_globals.h"

#if SIZEOF_SIZE_T > SIZEOF_ZEND_LONG
#error immutable_cache does not support platforms where size_t > zend_long
#endif

#define PHP_IMMUTABLE_CACHE_VERSION "6.0.2"
#define PHP_IMMUTABLE_CACHE_EXTNAME "immutable_cache"

PHP_IMMUTABLE_CACHE_API zend_bool immutable_cache_is_enabled(void);

extern zend_module_entry immutable_cache_module_entry;
#define immutable_cache_module_ptr &immutable_cache_module_entry

#define phpext_immutable_cache_ptr immutable_cache_module_ptr

#if defined(ZTS) && defined(COMPILE_DL_IMMUTABLE_CACHE)
ZEND_TSRMLS_CACHE_EXTERN();
#endif

#endif /* PHP_IMMUTABLE_CACHE_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim>600: noexpandtab sw=4 ts=4 sts=4 fdm=marker
 * vim<600: noexpandtab sw=4 ts=4 sts=4
 */

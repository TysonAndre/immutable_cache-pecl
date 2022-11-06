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
  | Copyright (c) 2006-2018 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Authors: Nikita Popov <nikic@php.net>                                |
  +----------------------------------------------------------------------+
 */

#ifndef IMMUTABLE_CACHE_STRINGS_H
#define IMMUTABLE_CACHE_STRINGS_H

#define IMMUTABLE_CACHE_STRINGS \
	X(access_time) \
	X(creation_time) \
	X(deletion_time) \
	X(hits) \
	X(info) \
	X(key) \
	X(mem_size) \
	X(mtime) \
	X(num_hits) \
	X(ref_count) \
	X(refs) \
	X(ttl) \
	X(type) \
	X(user) \
	X(value) \

#define X(str) extern zend_string *immutable_cache_str_ ## str;
	IMMUTABLE_CACHE_STRINGS
#undef X

#endif

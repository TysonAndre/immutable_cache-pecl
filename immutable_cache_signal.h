/*
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
  | Authors: Lucas Nealan <lucas@php.net>                                |
  +----------------------------------------------------------------------+

 */

#ifndef IMMUTABLE_CACHE_SIGNAL_H
#define IMMUTABLE_CACHE_SIGNAL_H

#include "immutable_cache.h"
#include "immutable_cache_php.h"

typedef struct immutable_cache_signal_entry_t {
	int signo;          /* signal number */
	int siginfo;        /* siginfo style handler calling */
	void* handler;      /* signal handler */
} immutable_cache_signal_entry_t;

typedef struct immutable_cache_signal_info_t {
	int installed;                  /* How many signals we've installed handles for */
	immutable_cache_signal_entry_t **prev;      /* Previous signal handlers */
} immutable_cache_signal_info_t;

void immutable_cache_set_signals(void);
void immutable_cache_shutdown_signals(void);

#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim>600: noexpandtab sw=4 ts=4 sts=4 fdm=marker
 * vim<600: noexpandtab sw=4 ts=4 sts=4
 */

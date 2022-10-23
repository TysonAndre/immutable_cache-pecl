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

   This software was contributed to PHP by Facebook Inc. in 2007.

   Future revisions and derivatives of this source code must acknowledge
   Facebook Inc. as the original contributor of this module by leaving
   this note intact in the source code.

   All other licensing and usage conditions are those of the PHP Group.
 */

/* Allows apc to install signal handlers and maintain signalling
 * to already registered handlers. Registers all signals that
 * coredump by default and unmaps the shared memory segment
 * before the coredump. Note: PHP module init is called before
 * signals are set by Apache and thus immutable_cache_set_signals should
 * be called in request init (RINIT)
 */

#include "immutable_cache.h"

#if HAVE_SIGACTION
#include <signal.h>
#include "immutable_cache_globals.h"
#include "immutable_cache_sma.h"
#include "immutable_cache_signal.h"
#include "immutable_cache_cache.h"

static immutable_cache_signal_info_t immutable_cache_signal_info = {0};

static int immutable_cache_register_signal(int signo, void (*handler)(int, siginfo_t*, void*));
static void immutable_cache_rehandle_signal(int signo, siginfo_t *siginfo, void *context);
static void immutable_cache_core_unmap(int signo, siginfo_t *siginfo, void *context);

extern immutable_cache_cache_t* immutable_cache_user_cache;

/* {{{ immutable_cache_core_unmap
 *  Coredump signal handler, detached from shm and calls previously installed handlers
 */
static void immutable_cache_core_unmap(int signo, siginfo_t *siginfo, void *context)
{
	if (immutable_cache_user_cache) {
		immutable_cache_sma_detach(immutable_cache_user_cache->sma);
	}
	immutable_cache_rehandle_signal(signo, siginfo, context);

#if !defined(WIN32) && !defined(NETWARE)
	kill(getpid(), signo);
#else
	raise(signo);
#endif
} /* }}} */


/* {{{ immutable_cache_rehandle_signal
 *  Call the previously registered handler for a signal
 */
static void immutable_cache_rehandle_signal(int signo, siginfo_t *siginfo, void *context)
{
	int i;
	immutable_cache_signal_entry_t p_sig = {0};

	for (i=0;  (i < immutable_cache_signal_info.installed && p_sig.signo != signo);  i++) {
		p_sig = *immutable_cache_signal_info.prev[i];
		if (p_sig.signo == signo) {
			if (p_sig.siginfo) {
				(*(void (*)(int, siginfo_t*, void*))p_sig.handler)(signo, siginfo, context);
			} else {
				(*(void (*)(int))p_sig.handler)(signo);
			}
		}
	}

} /* }}} */

/* {{{ immutable_cache_register_signal
 *  Set a handler for a previously installed signal and save so we can
 *  callback when handled
 */
static int immutable_cache_register_signal(int signo, void (*handler)(int, siginfo_t*, void*))
{
	struct sigaction sa;
	immutable_cache_signal_entry_t p_sig = {0};

	if (sigaction(signo, NULL, &sa) == 0) {
		if ((void*)sa.sa_handler == (void*)handler) {
			return SUCCESS;
		}

		if (sa.sa_handler != SIG_ERR && sa.sa_handler != SIG_DFL && sa.sa_handler != SIG_IGN) {
			p_sig.signo = signo;
			p_sig.siginfo = ((sa.sa_flags & SA_SIGINFO) == SA_SIGINFO);
			p_sig.handler = (void *)sa.sa_handler;

			immutable_cache_signal_info.prev = (immutable_cache_signal_entry_t **) perealloc(immutable_cache_signal_info.prev, (immutable_cache_signal_info.installed+1)*sizeof(immutable_cache_signal_entry_t *), 1);
			immutable_cache_signal_info.prev[immutable_cache_signal_info.installed] = (immutable_cache_signal_entry_t *) pemalloc(sizeof(immutable_cache_signal_entry_t), 1);
			*immutable_cache_signal_info.prev[immutable_cache_signal_info.installed++] = p_sig;
		} else {
			/* inherit flags and mask if already set */
			sigemptyset(&sa.sa_mask);
			sa.sa_flags = 0;
			sa.sa_flags |= SA_SIGINFO; /* we'll use a siginfo handler */
#if defined(SA_ONESHOT)
			sa.sa_flags = SA_ONESHOT;
#elif defined(SA_RESETHAND)
			sa.sa_flags = SA_RESETHAND;
#endif
		}
		sa.sa_handler = (void*)handler;

		if (sigaction(signo, &sa, NULL) < 0) {
			immutable_cache_warning("Error installing apc signal handler for %d", signo);
		}

		return SUCCESS;
	}
	return FAILURE;
} /* }}} */

/* {{{ immutable_cache_set_signals
 *  Install our signal handlers */
void immutable_cache_set_signals()
{
	if (immutable_cache_signal_info.installed == 0) {
#if defined(SIGUSR1) && defined(IMMUTABLE_CACHE_CLEAR_SIGNAL)
		immutable_cache_register_signal(SIGUSR1, immutable_cache_clear_cache);
#endif
		if (IMMUTABLE_CACHE_G(coredump_unmap)) {
			/* ISO C standard signals that coredump */
			immutable_cache_register_signal(SIGSEGV, immutable_cache_core_unmap);
			immutable_cache_register_signal(SIGABRT, immutable_cache_core_unmap);
			immutable_cache_register_signal(SIGFPE, immutable_cache_core_unmap);
			immutable_cache_register_signal(SIGILL, immutable_cache_core_unmap);
/* extended signals that coredump */
#ifdef SIGBUS
			immutable_cache_register_signal(SIGBUS, immutable_cache_core_unmap);
#endif
#ifdef SIGABORT
			immutable_cache_register_signal(SIGABORT, immutable_cache_core_unmap);
#endif
#ifdef SIGEMT
			immutable_cache_register_signal(SIGEMT, immutable_cache_core_unmap);
#endif
#ifdef SIGIOT
			immutable_cache_register_signal(SIGIOT, immutable_cache_core_unmap);
#endif
#ifdef SIGQUIT
			immutable_cache_register_signal(SIGQUIT, immutable_cache_core_unmap);
#endif
#ifdef SIGSYS
			immutable_cache_register_signal(SIGSYS, immutable_cache_core_unmap);
#endif
#ifdef SIGTRAP
			immutable_cache_register_signal(SIGTRAP, immutable_cache_core_unmap);
#endif
#ifdef SIGXCPU
			immutable_cache_register_signal(SIGXCPU, immutable_cache_core_unmap);
#endif
#ifdef SIGXFSZ
			immutable_cache_register_signal(SIGXFSZ, immutable_cache_core_unmap);
#endif
		}
	}
} /* }}} */

/* {{{ immutable_cache_set_signals
 *  cleanup signals for shutdown */
void immutable_cache_shutdown_signals()
{
	int i=0;
	if (immutable_cache_signal_info.installed > 0) {
		for (i=0; i < immutable_cache_signal_info.installed; i++) {
			free(immutable_cache_signal_info.prev[i]);
		}
		free(immutable_cache_signal_info.prev);
		immutable_cache_signal_info.installed = 0; /* just in case */
	}
}
/* }}} */

#endif  /* HAVE_SIGACTION */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim>600: noexpandtab sw=4 ts=4 sts=4 fdm=marker
 * vim<600: noexpandtab sw=4 ts=4 sts=4
 */

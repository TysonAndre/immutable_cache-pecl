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
  +----------------------------------------------------------------------+

   This software was contributed to PHP by Community Connect Inc. in 2002
   and revised in 2005 by Yahoo! Inc. to add support for PHP 5.1.
   Future revisions and derivatives of this source code must acknowledge
   Community Connect Inc. as the original contributor of this module by
   leaving this note intact in the source code.

   All other licensing and usage conditions are those of the PHP Group.

 */

#include "immutable_cache.h"
#include "immutable_cache_stack.h"

struct immutable_cache_stack_t {
	void** data;
	size_t capacity;
	size_t size;
};

immutable_cache_stack_t* immutable_cache_stack_create(size_t size_hint)
{
	immutable_cache_stack_t* stack = emalloc(sizeof(immutable_cache_stack_t));

	stack->capacity = (size_hint > 0) ? size_hint : 10;
	stack->size = 0;
	stack->data = emalloc(sizeof(void*) * stack->capacity);

	return stack;
}

void immutable_cache_stack_destroy(immutable_cache_stack_t* stack)
{
	if (stack != NULL) {
		efree(stack->data);
		efree(stack);
	}
}

void immutable_cache_stack_clear(immutable_cache_stack_t* stack)
{
	assert(stack != NULL);
	stack->size = 0;
}

void immutable_cache_stack_push(immutable_cache_stack_t* stack, void* item)
{
	assert(stack != NULL);
	if (stack->size == stack->capacity) {
		stack->capacity *= 2;
		stack->data = erealloc(stack->data, sizeof(void*) * stack->capacity);
	}
	stack->data[stack->size++] = item;
}

void* immutable_cache_stack_pop(immutable_cache_stack_t* stack)
{
	assert(stack != NULL && stack->size > 0);
	return stack->data[--stack->size];
}

void* immutable_cache_stack_top(immutable_cache_stack_t* stack)
{
	assert(stack != NULL && stack->size > 0);
	return stack->data[stack->size-1];
}

void* immutable_cache_stack_get(immutable_cache_stack_t* stack, size_t n)
{
	assert(stack != NULL && stack->size > n);
	return stack->data[n];
}

int immutable_cache_stack_size(immutable_cache_stack_t* stack)
{
	assert(stack != NULL);
	return stack->size;
}


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim>600: noexpandtab sw=4 ts=4 sts=4 fdm=marker
 * vim<600: noexpandtab sw=4 ts=4 sts=4
 */

<?php

/**
 * @generate-function-entries PHP_IMMUTABLE_CACHE_API
 * @generate-legacy-arginfo
 */

/**
 * Returns whether immutable_cache is usable in the current environment.
 */
function immutable_cache_enabled(): bool {}

/**
 * Cache a new variable in the immutable_cache data store.
 * Does nothing if the entry already exists.
 *
 * @param array|string $key
 */
function immutable_cache_add($key, mixed $value = UNKNOWN): array|bool {}

/**
 * Fetch a stored variable from the cache.
 *
 * @param array|string $key
 * @param bool $success set to true on success.
 */
function immutable_cache_fetch($key, &$success = null): mixed {}

/**
 * Returns whether a cache entry exists for the array/string $key
 * @param array|string $key
 */
function immutable_cache_exists($key): array|bool {}

/**
 * Retrieves information about the immutable_cache data store.
 *
 * @param bool $limited if true, then omit information about cache entries
 */
function immutable_cache_cache_info(bool $limited = false): array|false {}

/**
 * Get detailed information about the given cache key
 */
function immutable_cache_key_info(string $key): ?array {}

/**
 * Get immutable_cache Shared Memory Allocation information
 */
function immutable_cache_sma_info(bool $limited = false): array|false {}


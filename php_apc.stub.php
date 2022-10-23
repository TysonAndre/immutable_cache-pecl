<?php

/**
 * @generate-function-entries PHP_IMMUTABLE_CACHE_API
 * @generate-legacy-arginfo
 */

function immutable_cache_cache_info(bool $limited = false): array|false {}

function immutable_cache_key_info(string $key): ?array {}

function immutable_cache_sma_info(bool $limited = false): array|false {}

function immutable_cache_enabled(): bool {}

/** @param array|string $key */
function immutable_cache_add($key, mixed $value = UNKNOWN, int $ttl = 0): array|bool {}

/**
 * @param array|string $key
 * @param bool $success
 */
function immutable_cache_fetch($key, &$success = null): mixed {}

/** @param array|string $key */
function immutable_cache_exists($key): array|bool {}

<?php

/**
 * @generate-function-entries PHP_APCU_API
 * @generate-legacy-arginfo
 */

function apcu_cache_info(bool $limited = false): array|false {}

function apcu_key_info(string $key): ?array {}

function apcu_sma_info(bool $limited = false): array|false {}

function apcu_enabled(): bool {}

/** @param array|string $key */
function apcu_add($key, mixed $value = UNKNOWN, int $ttl = 0): array|bool {}

/**
 * @param array|string $key
 * @param bool $success
 */
function apcu_fetch($key, &$success = null): mixed {}

/** @param array|string $key */
function apcu_exists($key): array|bool {}

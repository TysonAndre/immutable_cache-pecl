immutable\_cache
================

[![Build Status](https://github.com/TysonAndre/immutable_cache-pecl/actions/workflows/config.yml/badge.svg?branch=main)](https://github.com/TysonAndre/immutable_cache-pecl/actions/workflows/config.yml?query=branch%3Amain)
[![Build status (Windows)](https://ci.appveyor.com/api/projects/status/7kccfd2a5i4q58ku/branch/main?svg=true)](https://ci.appveyor.com/project/TysonAndre/immutable-cache-pecl/branch/main)


**This is currently a prototype. Error handling and API design will likely change in subsequent releases. Features from APCu not mentioned in this README are likely unsupported and dropped later. There may be bugs.**

This is a work in progress PECL repo adding functionality similar to APCu,
but with immutable values (an immutable serialized copy of mutable values is stored in the serializer)

This is a fork of the original [APCu PECL](https://github.com/krakjoe/apcu)

Features:

- Returns the original persistent strings/arrays rather than a copy (for arrays that don't contain objects or PHP references)

  (not possible in a **mutable** in-memory pool with [APCu](https://github.com/krakjoe/apcu) due to eviction always being possible in APCu)

  This is instantaneous regardless of how large the array or strings are.
- Reuses immutable values if they were fetched from immutable_cache and stored again.

Planned but unimplemented features:

- Aggressively deduplicate values across cache entries (not possible in APCu due to the need to cache entries separately)

Benefits:

- Can be certain that a value added to the cache doesn't get removed.
- Can efficiently fetch entire immutable arrays or strings without copying or using extra memory
  in cases where the serializer is not required.

  (apcu needs to make copies of all strings and arrays in case of a cache clear during a request)

Caveats:

- APCu will also clear the shared memory cache as a way to recover from deadlocks or processes crashing while holding the shared memory lock, which should be rare.
  immutable_cache will never clear the shared memory cache.

Features
========

`immutable_cache` is an in-memory key-value store for PHP. Keys are of type string and values can be any PHP variables.

`immutable_cache` only supports userland caching of immutable representations of keys.

### API reference

This provides an API similar to a subset of the functions in https://php.net/apcu , but does not allow for modifying, deleting, incrementing, etc. on a created key.

When storing mutable values such as objects, it will instead call PHP's serialize() on the entire object.
Each retrieval will call unserialize() and return different values when the underlying value is mutable.

```php
<?php
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
```

### Ini settings

Similar to https://www.php.net/manual/en/apcu.configuration.php

- `immutable_cache.enabled` (bool, defaults to 1(on))
- `immutable_cache.enable_cli` (bool, defaults to 0(off))
- `immutable_cache.shm_size` (defaults to 32M)
- `immutable_cache.entries_hint` (size of the hash table. If this is too small, uses of the immutable cache will take longer from traversing a longer linked list to find the entries)
- `immutable_cache.serializer` (currently `php` or `default`. Defaults to `default` (Assumes the bugs with `serializer=default` were fixed in the upstream APCu 5.1.20 release). This is used to serialize data that doesn't have an immutable representation (e.g. objects, references (for now, it isn't converted)))

This is an immutable cache where entries can't be removed, so there is no need for ttl or gc_ttl.

### Benefits

Immutability allows `immutable_cache` to keep strings and arrays in shared memory, without making a copy.
Different processes (e.g. from apache worker pools) or threads (in ZTS builds) would
all be performing read-only access to the same constant arrays (like they would with opcache)

Benchmarks
==========

Note that when the data is completely immutable (i.e. does not need to call the php serializer for anything),
it can be stored in shared memory and retrieved from
`immutable_cache` without any modification (e.g. very large arrays, large strings, etc),
and would have the same throughput.

See [`benchmark_shm.php`](./benchmark_shm.php)

```
E.g. to retrieve multiple versions of the fake cached config
{"key0":"myValue0","key1":"myValue1","key2":"myValue2","key3":"myValue3","key4":"myValue4","key5":"myValue5","key6":"myValue6","key7":"myValue7"}
as a php array with 4 processes, repeatedly (with immutable_cache.enable_cli=1, immutable_cache.enabled=1, etc).

APCu            Elapsed: 0.339511 throughput    1178166 / second
APCu            Elapsed: 0.342087 throughput    1169294 / second
APCu            Elapsed: 0.424223 throughput     942901 / second
APCu            Elapsed: 0.424893 throughput     941413 / second
immutable_cache Elapsed: 0.109808 throughput    3642719 / second
immutable_cache Elapsed: 0.117099 throughput    3415902 / second
immutable_cache Elapsed: 0.099313 throughput    4027680 / second
immutable_cache Elapsed: 0.105851 throughput    3778904 / second
```

Installing
==========

## Compile immutable_cache in Linux

```
$ phpize
$ ./configure
$ make
$ make test
$ make install
```

Add the following line to your php.ini

```
extension=immutable_cache.so
```

## Compile immutable_cache in Windows

See https://wiki.php.net/internals/windows/stepbystepbuild_sdk_2#building_pecl_extensions and .appveyor.yml for how to build this.

Reporting Bugs
=============

If you believe you have found a bug in `immutable_cache`, please open an issue: Include in your report *minimal, executable, reproducing code*.

Minimal: reduce your problem to the smallest amount of code possible; This helps with hunting the bug, but also it helps with integration and regression testing once the bug is fixed.

Executable: include all the information required to execute the example code, code snippets are not helpful.

Reproducing: some bugs don't show themselves on every execution, that's fine, mention that in the report and give an idea of how often you encounter the bug.

__It is impossible to help without reproducing code, bugs that are opened without reproducing code will be closed.__

Please include version and operating system information in your report.

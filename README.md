immutable\_cache
================

[![Build Status](https://github.com/TysonAndre/immutable_cache-pecl/actions/workflows/config.yml/badge.svg?branch=main)](https://github.com/TysonAndre/immutable_cache-pecl/actions/workflows/config.yml?query=branch%3Amain)
[![Build status (Windows)](https://ci.appveyor.com/api/projects/status/7kccfd2a5i4q58ku/branch/main?svg=true)](https://ci.appveyor.com/project/TysonAndre/immutable-cache-pecl/branch/main)

This adds functionality similar to APCu,
but with immutable values (an immutable serialized copy of mutable values is stored in the serializer)

See [the installing section](#installing) for installation instructions.

This is a fork of the original [APCu PECL](https://github.com/krakjoe/apcu)

Features:

- Returns the original persistent strings/arrays rather than a copy (for arrays that don't contain objects or PHP references)

  (not possible in a **mutable** in-memory pool with [APCu](https://github.com/krakjoe/apcu) due to eviction always being possible in APCu)

  This is instantaneous regardless of how large the array or strings are.
- Reuses immutable values if they were fetched from immutable_cache and stored again.

Planned but unimplemented features:

- Aggressively deduplicate values across cache entries (not possible in APCu due to the need to cache entries separately)

Benefits:

- You can be certain that a value added to the cache doesn't get removed.
- You can efficiently fetch entire immutable arrays or strings without copying or using extra memory
  in cases where the serializer is not required.

  (APCu needs to make copies of all strings and arrays in case of a cache clear during a request)

Caveats:

- APCu will also clear the shared memory cache as a way to recover from deadlocks or processes crashing while holding the shared memory lock, which should be rare (e.g. a process getting killed or crashing while copying values).
  immutable_cache will never clear the shared memory cache.

Features
========

`immutable_cache` is an in-memory key-value store for PHP. Keys are of type string and values can be any PHP variables.

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
- `immutable_cache.shm_size` (defaults to 256M)
- `immutable_cache.entries_hint` (size of the hash table. If this is too small, uses of the immutable cache will take longer from traversing a longer linked list to find the entries)
- `immutable_cache.serializer` (currently `php` or `default`. Defaults to `default` (Assumes the bugs with `serializer=default` were fixed in the upstream APCu 5.1.20 release). This is used to serialize data that doesn't have an immutable representation (e.g. objects, references (for now, it isn't converted)))



This is an immutable cache where entries can't be removed, so there is no need for ttl or gc_ttl.

### Benefits

Immutability allows `immutable_cache` to keep strings and arrays in shared memory, without making a copy.
Different processes (e.g. from apache worker pools) or threads (in ZTS builds) would
all be performing read-only access to the same constant arrays (like they would with opcache)

Optimizations
=============

### Reducing memory usage

To reduce the total memory this uses, compile this with support for the igbinary PECL and enable `immutable_cache.serializer=igbinary`. See https://github.com/igbinary/igbinary#igbinary and https://github.com/igbinary/igbinary#installing

1. Install `igbinary`
2. Compile and install `immutable_cache`, configuring it `configure --enable-immutable-cache-igbinary`
3. Set `immutable_cache.serializer=igbinary` in your php.ini settings.

If this is done properly, `phpinfo()` will show igbinary as an available serializer.

Note that arrays of non-objects/non-references will take longer to decode when using `php` or `igbinary` (faster than `php` in most cases) because they need to be unserialized.

### Miscellaneous

When using `immutable_cache.serializer=default`, `immutable_cache` will reuse arrays/strings that were fetched from `immutable_cache` with `immutable_cache_fetch` (i.e. in the shared memory region managed by `immutable_cache`), because they're guaranteed to be immutable.

So for this example, `myarrayclone` is using very little memory apart from managing the cache entry.

```php
<?php
printf("immutable_cache.serializer=%s\n", ini_get('immutable_cache.serializer'));
$array = [];
for ($i = 0; $i < 16; $i++) {
    $array["key$i"] = "value$i";
}

immutable_cache_add('myarray', $array);
immutable_cache_add('myarrayclone', immutable_cache_fetch($array));
foreach (immutable_cache_cache_info()['cache_list'] as $entry) {
    printf("%12s memory usage: %d bytes\n", $entry['info'], $entry['mem_size']);
}
/*
immutable_cache.serializer=default
     myarray memory usage: 1816 bytes
myarrayclone memory usage: 160 bytes
 */
```

Note that opcache does not expose any APIs (that I know of) to check if an immutable array/string is from opcache's shared memory, which would allow further memory sharing.

### Preloading

Similarly to APCu, `immutable_cache.preload=/path/to/directory` can point to files in `/path/to/directory/*.data` containing PHP [serialize()d](https://www.php.net/serialize) data to unserialize before running the application.

This can be useful if you have large objects you want in the cache before a web server starts responding to requests.

Benchmarks
==========

Note that when the data is completely immutable (i.e. does not need to call the php serializer for anything and `immutable_cache.serializer=default` (default) is used),
values can be stored in shared memory and retrieved in constant time from
`immutable_cache` without any modification (e.g. very large arrays, large strings, etc).
i.e. these benchmarks have the same throughput regardless of the size of the data.

Benchmarks were run with opcache enabled in php 8.2 on a computer with 4 physical cores and 8 logical cores (with hyperthreading).

### Fetching an associative array of size 1000

[`BENCHMARK_N=40000 BENCHMARK_ARRAY_SIZE=1000 php benchmark_shm.php`](./benchmark_shm.php)
using `immutable_cache.serializer=default`.

This benchmarks unserializing a 1000 element array of the form `{...,"key999":"myValue999"}` with 4 concurrently running processes,
and is around 240 times faster than APCu.

```
apc.serializer = default
immutable_cache.serializer = default
immutable_cache Elapsed: 0.007205 throughput    5551667 / second
immutable_cache Elapsed: 0.007373 throughput    5425447 / second
immutable_cache Elapsed: 0.007700 throughput    5195134 / second
immutable_cache Elapsed: 0.007252 throughput    5515629 / second
APCu            Elapsed: 1.787497 throughput      22378 / second
APCu            Elapsed: 1.788314 throughput      22367 / second
APCu            Elapsed: 1.791437 throughput      22328 / second
APCu            Elapsed: 1.796633 throughput      22264 / second
```

### Fetching an associative array of size 8

Running [`benchmark_shm.php`](./benchmark_shm.php), this is around 3.7 times as fast

```
E.g. to retrieve multiple versions of the fake cached config
{"key0":"myValue0","key1":"myValue1","key2":"myValue2","key3":"myValue3","key4":"myValue4","key5":"myValue5","key6":"myValue6","key7":"myValue7"}
as a php array with 4 processes, repeatedly (with immutable_cache.enable_cli=1, immutable_cache.enabled=1, etc).

apc.serializer = default
immutable_cache.serializer = default
immutable_cache Elapsed: 0.083897 throughput    4767745 / second
immutable_cache Elapsed: 0.083710 throughput    4778405 / second
immutable_cache Elapsed: 0.085307 throughput    4688932 / second
immutable_cache Elapsed: 0.085654 throughput    4669966 / second
APCu            Elapsed: 0.317105 throughput    1261412 / second
APCu            Elapsed: 0.320551 throughput    1247850 / second
APCu            Elapsed: 0.322219 throughput    1241391 / second
APCu            Elapsed: 0.331782 throughput    1205612 / second
```

### Fetching a string of size 100000

`BENCHMARK_ARRAY_SIZE=100000 BENCHMARK_USE_STRING_INSTEAD=1 php benchmark_shm.php`
was 20 times faster for retrieving strings of size 100000 with 4 processes running in parallel.
(APCu must copy the entire string when retrieving strings in case of a cache clear.)

```
immutable_cache Elapsed: 0.081627 throughput    4900352 / second
immutable_cache Elapsed: 0.082891 throughput    4825640 / second
immutable_cache Elapsed: 0.081830 throughput    4888166 / second
immutable_cache Elapsed: 0.082876 throughput    4826462 / second
APCu            Elapsed: 1.655446 throughput     241627 / second
APCu            Elapsed: 1.657670 throughput     241303 / second
APCu            Elapsed: 1.662154 throughput     240652 / second
APCu            Elapsed: 1.684609 throughput     237444 / second
```

### Fetching a scalar

`BENCHMARK_USE_SCALAR_INSTEAD=1 php benchmark_shm.php` was used to fetch an integer from
16 different cache keys. Because of the use of fine-grained locks in immutable_cache to avoid contention,
there was still a performance improvement (around 2.5x) compared to APCu.

```
immutable_cache Elapsed: 0.078275 throughput    5110164 / second
immutable_cache Elapsed: 0.080960 throughput    4940725 / second
immutable_cache Elapsed: 0.080830 throughput    4948671 / second
immutable_cache Elapsed: 0.080430 throughput    4973270 / second
APCu            Elapsed: 0.194570 throughput    2055819 / second
APCu            Elapsed: 0.198471 throughput    2015405 / second
APCu            Elapsed: 0.198798 throughput    2012093 / second
APCu            Elapsed: 0.199741 throughput    2002590 / second
```

### Fetching with `immutable_cache.protect_memory=1`

`php -d immutable_cache.protect_memory=1 benchmark_shm.php` was used to run immutable_cache with the option to protect memory for debugging (this will mark the memory as read-only when not in use.
A side effect of this is that

1. Read operations stop incrementing counters for cache hits, last access times, etc.
   So those operations are sped up because processors do less work and caches aren't invalidated by changing these.
2. Write operations are slower, because mprotect is called.

```
Testing string->string array of size 8 with 4 forked processes
Note that the immutable array and strings part of shared memory in immutable_cache, where apcu must make copies of strings and arrays to account for eviction (and php must free them)
{"key0":"myValue0","key1":"myValue1","key2":"myValue2","key3":"myValue3","key4":"myValue4","key5":"myValue5","key6":"myValue6","key7":"myValue7"}

immutable_cache Elapsed: 0.053691 throughput    7450103 / second
immutable_cache Elapsed: 0.056164 throughput    7121941 / second
immutable_cache Elapsed: 0.054956 throughput    7278549 / second
immutable_cache Elapsed: 0.059204 throughput    6756267 / second
APCu            Elapsed: 0.318515 throughput    1255828 / second
APCu            Elapsed: 0.314734 throughput    1270913 / second
APCu            Elapsed: 0.320162 throughput    1249368 / second
APCu            Elapsed: 0.327550 throughput    1221189 / second
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

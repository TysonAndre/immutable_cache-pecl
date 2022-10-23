immutable\_cache
================

**Not working yet**
This is a work in progress PECL repo adding functionality similar to APCu,
but with immutable values (an immutable serialized copy of mutable values is stored in the serializer)

This is a fork of the original APCu source code from https://github.com/krakjoe/apcu

Goals:

- Return the original persistent strings/arrays rather than a copy, on supported platforms.

  (Not implemented yet, not possible in a **mutable** in-memory pool with APCu)
- Aggressively deduplicate values across cache entries (not possible in APCu due to the need to cache entries separately)

Benefits:

- Can be certain that a value added to the cache doesn't get removed
- Once implemented, can efficiently load entire immutable arrays or strings in cases where the serializer is not required.

Features
========

ImmutableCache is an in-memory key-value store for PHP. Keys are of type string and values can be any PHP variables.

APCu only supports userland caching of immutable representations of variables.

See [`php_immutable_cache.stub.php`](./php_immutable_cache.stub.php) for the api.

Reporting Bugs
=============

If you believe you have found a bug in `immutable_cache`, please open an issue: Include in your report *minimal, executable, reproducing code*.

Minimal: reduce your problem to the smallest amount of code possible; This helps with hunting the bug, but also it helps with integration and regression testing once the bug is fixed.

Executable: include all the information required to execute the example code, code snippets are not helpful.

Reproducing: some bugs don't show themselves on every execution, that's fine, mention that in the report and give an idea of how often you encounter the bug.

__It is impossible to help without reproducing code, bugs that are opened without reproducing code will be closed.__

Please include version and operating system information in your report.

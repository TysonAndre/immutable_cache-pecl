#!/usr/bin/env bash
# -x Exit immediately if any command fails
# -e Echo all commands being executed.
# -u fail for undefined variables
set -xeu
echo "Run tests in docker"
php --version
php --ini
cp ci/run-tests-parallel.php run-tests.php
export REPORT_EXIT_STATUS=1

echo "running tests without valgrind in a debug build"
make test TESTS="-j$(nproc) --show-diff -d immutable_cache.serializer=default tests"
make test TESTS="-j$(nproc) --show-diff -d immutable_cache.serializer=php tests"
make test TESTS="-j$(nproc) --show-diff -d immutable_cache.serializer=default -d opcache.enable_cli=1 tests"
make test TESTS="-j$(nproc) --show-diff -d immutable_cache.serializer=php -d opcache.enable_cli=1 tests"

echo "re-running tests in valgrind"
make test TESTS="-j$(nproc) -P -q --show-diff -m --show-mem"
echo "Test that package.xml is valid"
pecl package

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
echo "Test with default representation, creating arrays in shared memory when possible"
make test TESTS="-j$(nproc) --show-diff -d immutable_cache.serializer=default tests"
echo "Test with storing serialize() in shared memory when possible"
make test TESTS="-j$(nproc) --show-diff -d immutable_cache.serializer=php tests"
echo "Test with opcache enabled and immutable arrays in memory when possible"
make test TESTS="-j$(nproc) --show-diff -d immutable_cache.serializer=default -d opcache.enable_cli=1 tests"
echo "Test with opcache enabled and serialize()"
make test TESTS="-j$(nproc) --show-diff -d immutable_cache.serializer=php -d opcache.enable_cli=1 tests"
echo "Test with protect_memory on the immutable strings/objects in shared memory"
make test TESTS="-j$(nproc) --show-diff -d immutable_cache.serializer=default -d immutable_cache.protect_memory=1 tests"

PECL_EXTENSION_DIR=$(php -r "echo ini_get('extension_dir');")
echo "Pecl extensions are installed in $PECL_EXTENSION_DIR"
igbinary_path="$PECL_EXTENSION_DIR/igbinary.so"
if [[ -f "$igbinary_path" ]]; then
	cp "$igbinary_path" modules/
	php -d extension=modules/immutable_cache.so -d extension=modules/igbinary.so -d immutable_cache.serializer=igbinary -r 'phpinfo();'
	make test TESTS="-j$(nproc) --show-diff -d extension=$PWD/modules/igbinary.so -d immutable_cache.serializer=igbinary tests"
fi

echo "Test that package.xml is valid"
pecl package

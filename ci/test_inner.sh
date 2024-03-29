#!/usr/bin/env bash
# -x Exit immediately if any command fails
# -e Echo all commands being executed.
# -u fail for undefined variables
set -xeu
echo "Run tests in docker"
php --version
php --ini

PECL_EXTENSION_DIR=$(php -r "echo ini_get('extension_dir');")
echo "Pecl extensions are installed in $PECL_EXTENSION_DIR"
igbinary_path="$PECL_EXTENSION_DIR/igbinary.so"
if [[ -f "$igbinary_path" ]]; then
	cp "$igbinary_path" modules/
	export TEST_PHP_ARGS="-n -d extension=igbinary.so -d extension=$PWD/modules/immutable_cache.so"
	php $TEST_PHP_ARGS -d immutable_cache.serializer=igbinary -r 'phpinfo();'
else
	export TEST_PHP_ARGS="-n -d extension=$PWD/modules/immutable_cache.so"
fi

php $TEST_PHP_ARGS -m

cp ci/run-tests-parallel.php run-tests.php
export REPORT_EXIT_STATUS=1
echo "Test with default representation, creating arrays in shared memory when possible"
php run-tests.php -j$(nproc) -P --show-diff -d immutable_cache.serializer=default tests
echo "Test with storing serialize() in shared memory when possible"
php run-tests.php -j$(nproc) -P --show-diff -d immutable_cache.serializer=php tests
echo "Test with opcache enabled and immutable arrays in memory when possible"
php run-tests.php -j$(nproc) -P --show-diff -d immutable_cache.serializer=default -d opcache.enable_cli=1 tests
echo "Test with opcache enabled and serialize()"
php run-tests.php -j$(nproc) -P --show-diff -d immutable_cache.serializer=php -d opcache.enable_cli=1 tests
echo "Test with protect_memory on the immutable strings/objects in shared memory"
php run-tests.php -j$(nproc) -P --show-diff -d immutable_cache.serializer=default -d immutable_cache.protect_memory=1 tests

if [[ -f "$igbinary_path" ]]; then
	echo "Test with igbinary serializer"
	php run-tests.php -j$(nproc) -P --show-diff -d immutable_cache.serializer=igbinary tests
fi

echo "Test that package.xml is valid"
pecl package

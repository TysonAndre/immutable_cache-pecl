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
EXTRA_TEST_PHP_ARGS=''
igbinary_path="$PECL_EXTENSION_DIR/igbinary.so"
if [[ -f "$igbinary_path" ]]; then
	cp "$igbinary_path" modules/
	php -d extension=modules/immutable_cache.so -d extension=modules/igbinary.so -d immutable_cache.serializer=igbinary -r 'phpinfo();'
	EXTRA_TEST_PHP_ARGS="-d extension=$PWD/modules/igbinary.so"
fi

cp ci/run-tests-parallel.php run-tests.php
export REPORT_EXIT_STATUS=1
echo "Test with default representation, creating arrays in shared memory when possible"
make test TESTS="-j$(nproc) --show-diff -d immutable_cache.serializer=default tests $EXTRA_TEST_PHP_ARGS"
echo "Test with storing serialize() in shared memory when possible"
make test TESTS="-j$(nproc) --show-diff -d immutable_cache.serializer=php tests $EXTRA_TEST_PHP_ARGS"
echo "Test with opcache enabled and immutable arrays in memory when possible"
make test TESTS="-j$(nproc) --show-diff -d immutable_cache.serializer=default -d opcache.enable_cli=1 tests $EXTRA_TEST_PHP_ARGS"
echo "Test with opcache enabled and serialize()"
make test TESTS="-j$(nproc) --show-diff -d immutable_cache.serializer=php -d opcache.enable_cli=1 tests $EXTRA_TEST_PHP_ARGS"
echo "Test with protect_memory on the immutable strings/objects in shared memory"
make test TESTS="-j$(nproc) --show-diff -d immutable_cache.serializer=default -d immutable_cache.protect_memory=1 tests $EXTRA_TEST_PHP_ARGS"

if [[ -f "$igbinary_path" ]]; then
	echo "Test with igbinary serializer"
	make test TESTS="-j$(nproc) --show-diff -d immutable_cache.serializer=igbinary tests $EXTRA_TEST_PHP_ARGS"
fi

echo "Test that package.xml is valid"
pecl package

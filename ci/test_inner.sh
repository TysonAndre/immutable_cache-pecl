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
make test TESTS="-j$(nproc) --show-diff -d immutable_cache.serializer=default tests"
make test TESTS="-j$(nproc) --show-diff -d immutable_cache.serializer=php tests"
make test TESTS="-j$(nproc) --show-diff -d immutable_cache.serializer=default -d opcache.enable_cli=1 tests"
make test TESTS="-j$(nproc) --show-diff -d immutable_cache.serializer=php -d opcache.enable_cli=1 tests"

PECL_EXTENSION_DIR=$(php -r "echo ini_get('extension_dir');")
echo "Pecl extensions are installed in $PECL_EXTENSION_DIR"
igbinary_path="$PECL_EXTENSION_DIR/igbinary.so"
if [[ -f "$igbinary_path" ]]; then
	cp "$PECL_EXTENSION_DIR/igbinary.so" modules/
	php -d extension=modules/immutable_cache.so -d extension=modules/igbinary.so -d immutable_cache.serializer=igbinary -r 'phpinfo();'
	make test TESTS="-j$(nproc) --show-diff -d extension=$PWD/modules/igbinary.so -d immutable_cache.serializer=igbinary tests"
fi

echo "Test that package.xml is valid"
pecl package

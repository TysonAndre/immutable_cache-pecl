#!/usr/bin/env bash
if [[ $# < 1 || $# > 2 ]]; then
    echo "Usage: $0 PHP_VERSION [i386]" 1>&2
    echo "e.g. $0 8.0 php" 1>&2
    echo "The PHP_VERSION is the version of the php docker image to use" 1>&2
    exit 1
fi
# -x Exit immediately if any command fails
# -e Echo all commands being executed.
# -u fail for undefined variables
set -xeu
PHP_VERSION=$1
ARCHITECTURE=${2:-}

CFLAGS="-O2"

if [[ "$ARCHITECTURE" == i386 ]]; then
	echo "skip i386 is not the same as compiling on 64-bit with -m32 cflags for all word sizes"
	exit 0
	PHP_IMAGE="$ARCHITECTURE/php"
	DOCKER_IMAGE="immutable-cache-test-runner:$ARCHITECTURE-$PHP_VERSION"
	CFLAGS="$CFLAGS -m32"
else
	PHP_IMAGE="php"
	DOCKER_IMAGE="immutable-cache-test-runner:$PHP_VERSION"
fi

docker build --build-arg="CFLAGS=$CFLAGS" --build-arg="PHP_VERSION=$PHP_VERSION" --build-arg="PHP_IMAGE=$PHP_IMAGE" --tag="$DOCKER_IMAGE" -f ci/Dockerfile .
docker run --rm $DOCKER_IMAGE ci/test_inner.sh

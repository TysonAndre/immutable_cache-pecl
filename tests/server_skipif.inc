<?php
require_once(__DIR__ . '/skipif.inc');
if (getenv('USE_ZEND_ALLOC') === '0' && PHP_INT_SIZE === 4) {
    die('skip https://bugs.launchpad.net/ubuntu/+source/valgrind/+bug/1877963');
}
if (PHP_VERSION_ID < 70400) {
    die('skip server startup output in php 7.3 and earlier');
}

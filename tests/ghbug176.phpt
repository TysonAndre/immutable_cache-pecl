--TEST--
APC: GH Bug #176 preload_path segfaults with bad data
--SKIPIF--
<?php
require_once(dirname(__FILE__) . '/skipif.inc');
if (PHP_ZTS === 1) {
    die('skip PHP non-ZTS only');
}
if (getenv('USE_ZEND_ALLOC') === '0' && PHP_INT_SIZE === 4) {
    die('skip https://bugs.launchpad.net/ubuntu/+source/valgrind/+bug/1877963');
}
?>
--CONFLICTS--
server
--FILE--
<?php
include "server_test.inc";

$file = <<<FL
\$key = 'abc';
\$b = immutable_cache_exists(\$key);
var_dump(\$b);
if (\$b) {
	\$\$key = immutable_cache_fetch(\$key);
	var_dump(\$\$key);
}
FL;

$args = array(
	'immutable_cache.enabled=1',
	'immutable_cache.enable_cli=1',
	'immutable_cache.preload_path=' . dirname(__FILE__) . '/bad',
);

server_start($file, $args);

for ($i = 0; $i < 10; $i++) {
	run_test_simple();
}
echo 'done';
?>
--EXPECT--
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
done

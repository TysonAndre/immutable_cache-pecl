--TEST--
APC: Bug #61742 preload_path does not work due to incorrect string length (variant 1)
--SKIPIF--
<?php
    require_once(dirname(__FILE__) . '/skipif.inc');
	if(PHP_ZTS === 1) {
		die('skip PHP non-ZTS only');
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
	'immutable_cache.preload_path=' . dirname(__FILE__) . '/data',
);

$num_servers = 1;
server_start($file, $args);

for ($i = 0; $i < 10*3; $i++) {
	run_test_simple();
}
echo 'done';

?>
--EXPECT--
bool(true)
string(3) "123"
bool(true)
string(3) "123"
bool(true)
string(3) "123"
bool(true)
string(3) "123"
bool(true)
string(3) "123"
bool(true)
string(3) "123"
bool(true)
string(3) "123"
bool(true)
string(3) "123"
bool(true)
string(3) "123"
bool(true)
string(3) "123"
bool(true)
string(3) "123"
bool(true)
string(3) "123"
bool(true)
string(3) "123"
bool(true)
string(3) "123"
bool(true)
string(3) "123"
bool(true)
string(3) "123"
bool(true)
string(3) "123"
bool(true)
string(3) "123"
bool(true)
string(3) "123"
bool(true)
string(3) "123"
bool(true)
string(3) "123"
bool(true)
string(3) "123"
bool(true)
string(3) "123"
bool(true)
string(3) "123"
bool(true)
string(3) "123"
bool(true)
string(3) "123"
bool(true)
string(3) "123"
bool(true)
string(3) "123"
bool(true)
string(3) "123"
bool(true)
string(3) "123"
done

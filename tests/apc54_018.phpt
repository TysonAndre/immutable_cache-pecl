--TEST--
APC: Bug #59938 APCIterator fails with large user cache
--SKIPIF--
<?php
require_once(dirname(__FILE__) . '/skipif.inc');
if (PHP_OS == "WINNT") die("skip not on windows");
if (getenv('USE_ZEND_ALLOC') === '0') die("skip not for valgrind");
?>
--CONFLICTS--
server
--FILE--
<?php
include "server_test.inc";

$file = <<<FL
//to fill apc cache (~40MB):
for(\$i=0;\$i<10000;\$i++) {
    \$value = str_repeat(md5(microtime()), 100);
    immutable_cache_add('test-niko-asdfasdfasdfkjasdflkasjdfasf'.\$i, \$value);
}

//then later (usually after a few minutes) this won't work correctly:
\$it = new ImmutableCacheIterator('#^test-niko-asdfasdfasdfkjasdflkasjdfasf#');
var_dump(\$it->getTotalCount()); //returns 10000
var_dump(\$it->current()); //returns false on error
FL;

$args = array(
	'immutable_cache.enabled=1',
	'immutable_cache.enable_cli=1',
	'immutable_cache.shm_size=64M',
);

server_start($file, $args);

for ($i = 0; $i < 3; $i++) {
	run_test_simple();
}
echo 'done';
?>
--EXPECTF--
int(10000)
array(7) {
  ["type"]=>
  string(4) "user"
  ["key"]=>
  string(%d) "test-niko-asdfasdfasdfkjasdflkasjdfasf%d"
  ["value"]=>
  string(%d) "%s"
  ["num_hits"]=>
  int(0)
  ["creation_time"]=>
  int(%d)
  ["access_time"]=>
  int(%d)
  ["mem_size"]=>
  int(%d)
}
int(10000)
array(7) {
  ["type"]=>
  string(4) "user"
  ["key"]=>
  string(%d) "test-niko-asdfasdfasdfkjasdflkasjdfasf%d"
  ["value"]=>
  string(%d) "%s"
  ["num_hits"]=>
  int(0)
  ["creation_time"]=>
  int(%d)
  ["access_time"]=>
  int(%d)
  ["mem_size"]=>
  int(%d)
}
int(10000)
array(7) {
  ["type"]=>
  string(4) "user"
  ["key"]=>
  string(%d) "test-niko-asdfasdfasdfkjasdflkasjdfasf%d"
  ["value"]=>
  string(%d) "%s"
  ["num_hits"]=>
  int(0)
  ["creation_time"]=>
  int(%d)
  ["access_time"]=>
  int(%d)
  ["mem_size"]=>
  int(%d)
}
int(10000)
array(7) {
  ["type"]=>
  string(4) "user"
  ["key"]=>
  string(%d) "test-niko-asdfasdfasdfkjasdflkasjdfasf%d"
  ["value"]=>
  string(%d) "%s"
  ["num_hits"]=>
  int(0)
  ["creation_time"]=>
  int(%d)
  ["access_time"]=>
  int(%d)
  ["mem_size"]=>
  int(%d)
}
int(10000)
array(7) {
  ["type"]=>
  string(4) "user"
  ["key"]=>
  string(%d) "test-niko-asdfasdfasdfkjasdflkasjdfasf%d"
  ["value"]=>
  string(%d) "%s"
  ["num_hits"]=>
  int(0)
  ["creation_time"]=>
  int(%d)
  ["access_time"]=>
  int(%d)
  ["mem_size"]=>
  int(%d)
}
int(10000)
array(7) {
  ["type"]=>
  string(4) "user"
  ["key"]=>
  string(%d) "test-niko-asdfasdfasdfkjasdflkasjdfasf%d"
  ["value"]=>
  string(%d) "%s"
  ["num_hits"]=>
  int(0)
  ["creation_time"]=>
  int(%d)
  ["access_time"]=>
  int(%d)
  ["mem_size"]=>
  int(%d)
}
int(10000)
array(7) {
  ["type"]=>
  string(4) "user"
  ["key"]=>
  string(%d) "test-niko-asdfasdfasdfkjasdflkasjdfasf%d"
  ["value"]=>
  string(%d) "%s"
  ["num_hits"]=>
  int(0)
  ["creation_time"]=>
  int(%d)
  ["access_time"]=>
  int(%d)
  ["mem_size"]=>
  int(%d)
}
int(10000)
array(7) {
  ["type"]=>
  string(4) "user"
  ["key"]=>
  string(%d) "test-niko-asdfasdfasdfkjasdflkasjdfasf%d"
  ["value"]=>
  string(%d) "%s"
  ["num_hits"]=>
  int(0)
  ["creation_time"]=>
  int(%d)
  ["access_time"]=>
  int(%d)
  ["mem_size"]=>
  int(%d)
}
int(10000)
array(7) {
  ["type"]=>
  string(4) "user"
  ["key"]=>
  string(%d) "test-niko-asdfasdfasdfkjasdflkasjdfasf%d"
  ["value"]=>
  string(%d) "%s"
  ["num_hits"]=>
  int(0)
  ["creation_time"]=>
  int(%d)
  ["access_time"]=>
  int(%d)
  ["mem_size"]=>
  int(%d)
}
done

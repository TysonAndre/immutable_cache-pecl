--TEST--
Success parameters should respect property types
--SKIPIF--
<?php
require_once(dirname(__FILE__) . '/skipif.inc');
version_compare(PHP_VERSION, '7.4.0dev', '>=') or die('skip Requires PHP >= 7.4');
?>
--INI--
immutable_cache.enabled=1
immutable_cache.enable_cli=1
--FILE--
<?php

class Test {
    public bool $bool = false;
    public array $array = [];
}

$test = new Test;
immutable_cache_add('foo', 'bar');
immutable_cache_fetch('foo', $test->bool);
var_dump($test->bool);
try {
    immutable_cache_fetch('foo', $test->array);
} catch (Error $e) {
    echo $e->getMessage(), "\n";
}
var_dump($test->array);

?>
--EXPECTF--
bool(true)
Cannot assign %s to reference held by property Test::$array of type array
array(0) {
}

--TEST--
Should be able to pass references to strings to immutable_cache_fetch
--SKIPIF--
<?php require_once(dirname(__FILE__) . '/skipif.inc'); ?>
--INI--
immutable_cache.enabled=1
immutable_cache.enable_cli=1
--FILE--
<?php
$array = ['foo', 'bar'];
var_dump(immutable_cache_add('foo', 'baz'));
var_dump(immutable_cache_fetch($array));
var_dump(error_get_last());
array_walk($array, function(&$item) {});
var_dump($array);
var_dump(immutable_cache_fetch($array));
var_dump(error_get_last());
?>
--EXPECT--
bool(true)
array(1) {
  ["foo"]=>
  string(3) "baz"
}
NULL
array(2) {
  [0]=>
  string(3) "foo"
  [1]=>
  string(3) "bar"
}
array(1) {
  ["foo"]=>
  string(3) "baz"
}
NULL

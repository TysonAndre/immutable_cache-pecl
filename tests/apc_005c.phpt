--TEST--
APC: immutable_cache_add/fetch with arrays with two object references
--SKIPIF--
<?php require_once(dirname(__FILE__) . '/skipif.inc'); ?>
--INI--
immutable_cache.enabled=1
immutable_cache.enable_cli=1
--FILE--
<?php

$o = new stdClass();
$foo = array(&$o, &$o);

var_dump($foo);

immutable_cache_add('foo',$foo);

$bar = immutable_cache_fetch('foo');
var_dump($foo);
// $bar[0] should be a reference to $bar[1].
var_dump($bar);
$bar[0] = 'roh';
var_dump($bar);
?>
===DONE===
--EXPECT--
array(2) {
  [0]=>
  &object(stdClass)#1 (0) {
  }
  [1]=>
  &object(stdClass)#1 (0) {
  }
}
array(2) {
  [0]=>
  &object(stdClass)#1 (0) {
  }
  [1]=>
  &object(stdClass)#1 (0) {
  }
}
array(2) {
  [0]=>
  &object(stdClass)#2 (0) {
  }
  [1]=>
  &object(stdClass)#2 (0) {
  }
}
array(2) {
  [0]=>
  &string(3) "roh"
  [1]=>
  &string(3) "roh"
}
===DONE===

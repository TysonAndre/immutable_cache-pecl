--TEST--
APC: immutable_cache_add/fetch with arrays of objects
--SKIPIF--
<?php require_once(dirname(__FILE__) . '/skipif.inc'); ?>
--INI--
immutable_cache.enabled=1
immutable_cache.enable_cli=1
--FILE--
<?php

$foo = array(new stdclass(), new stdclass());

var_dump($foo);

immutable_cache_add('foo',$foo);

$bar = immutable_cache_fetch('foo');
var_dump($foo);
var_dump($bar);

?>
===DONE===
--EXPECT--
array(2) {
  [0]=>
  object(stdClass)#1 (0) {
  }
  [1]=>
  object(stdClass)#2 (0) {
  }
}
array(2) {
  [0]=>
  object(stdClass)#1 (0) {
  }
  [1]=>
  object(stdClass)#2 (0) {
  }
}
array(2) {
  [0]=>
  object(stdClass)#3 (0) {
  }
  [1]=>
  object(stdClass)#4 (0) {
  }
}
===DONE===

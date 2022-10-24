--TEST--
immutable_cache_fetch should work for multiple reference groups
--SKIPIF--
<?php require_once(dirname(__FILE__) . '/skipif.inc'); ?>
--INI--
immutable_cache.enabled=1
immutable_cache.enable_cli=1
--FILE--
<?php

$x = [];
$y = [];
$array = [&$x, &$x, &$y, &$y];
immutable_cache_add("ary", $array);
$copy = immutable_cache_fetch("ary");
$copy[0][1] = new stdClass();
var_dump($array);
var_dump($copy);

?>
--EXPECT--
array(4) {
  [0]=>
  &array(0) {
  }
  [1]=>
  &array(0) {
  }
  [2]=>
  &array(0) {
  }
  [3]=>
  &array(0) {
  }
}
array(4) {
  [0]=>
  &array(1) {
    [1]=>
    object(stdClass)#1 (0) {
    }
  }
  [1]=>
  &array(1) {
    [1]=>
    object(stdClass)#1 (0) {
    }
  }
  [2]=>
  &array(0) {
  }
  [3]=>
  &array(0) {
  }
}

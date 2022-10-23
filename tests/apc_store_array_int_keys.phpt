--TEST--
immutable_cache_add() with int keys in array should convert them to string
--SKIPIF--
<?php
require_once(__DIR__ . '/skipif.inc');
?>
--INI--
immutable_cache.enabled=1
immutable_cache.enable_cli=1
--FILE--
<?php

var_dump(immutable_cache_add(["123" => "test"]));
// no store option
var_dump(immutable_cache_add(["123" => "test"]));

?>
--EXPECT--
array(0) {
}
array(1) {
  [123]=>
  int(-1)
}

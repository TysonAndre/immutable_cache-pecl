--TEST--
apcu_add() with int keys in array should convert them to string
--SKIPIF--
<?php
require_once(__DIR__ . '/skipif.inc');
?>
--INI--
apc.enabled=1
apc.enable_cli=1
--FILE--
<?php

var_dump(apcu_add(["123" => "test"]));
// no store option
var_dump(apcu_add(["123" => "test"]));

?>
--EXPECT--
array(0) {
}
array(0) {
}
array(1) {
  [123]=>
  int(-1)
}

--TEST--
Serialization edge cases
--SKIPIF--
<?php require_once(dirname(__FILE__) . '/skipif.inc'); ?>
--INI--
apc.enabled=1
apc.enable_cli=1
--FILE--
<?php

echo "GLOBALS:\n";
$foo = 1;
immutable_cache_add("key1", $GLOBALS);
$globals = immutable_cache_fetch("key1");
var_dump($globals['foo']);

echo "Object referential identity:\n";
$obj = new stdClass;
$obj2 = new stdClass;
$obj2->obj = $obj;
$ary = [$obj, $obj2];
immutable_cache_add("key2", $ary);
// $obj and $obj2->obj should have the same ID
var_dump(immutable_cache_fetch("key2"));

echo "Array next free element:\n";
$ary = [0, 1];
unset($ary[1]);
immutable_cache_add("key3", $ary);
$ary = immutable_cache_fetch("key3");
// This should use key 1 rather than 2, as
// nextFreeElement should not be preserved (serialization does not)
$ary[] = 1;
var_dump($ary);

echo "Resources:\n";
immutable_cache_add("key4", fopen(__FILE__, "r"));

?>
--EXPECTF--
GLOBALS:
int(1)
Object referential identity:
array(2) {
  [0]=>
  object(stdClass)#3 (0) {
  }
  [1]=>
  object(stdClass)#4 (1) {
    ["obj"]=>
    object(stdClass)#3 (0) {
    }
  }
}
Array next free element:
array(2) {
  [0]=>
  int(0)
  [1]=>
  int(1)
}
Resources:

Warning: immutable_cache_add(): Cannot store resources in apcu cache in %s on line %d

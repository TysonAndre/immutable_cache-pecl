--TEST--
APC: immutable_cache_exists
--SKIPIF--
<?php require_once(dirname(__FILE__) . '/skipif.inc'); ?>
--INI--
apc.enabled=1
apc.enable_cli=1
--FILE--
<?php
$kyes = "testkey";
$kno  = "keytest";
immutable_cache_add($kyes, 1);
var_dump(immutable_cache_exists($kyes));
var_dump(immutable_cache_exists($kno));
var_dump(immutable_cache_exists([$kyes, $kno]));
?>
===DONE===
--EXPECT--
bool(true)
bool(false)
array(1) {
  ["testkey"]=>
  bool(true)
}
===DONE===

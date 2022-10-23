--TEST--
APC: apcu_exists
--SKIPIF--
<?php require_once(dirname(__FILE__) . '/skipif.inc'); ?>
--INI--
apc.enabled=1
apc.enable_cli=1
--FILE--
<?php
$kyes = "testkey";
$kno  = "keytest";
apcu_add($kyes, 1);
var_dump(apcu_exists($kyes));
var_dump(apcu_exists($kno));
var_dump(apcu_exists([$kyes, $kno]));
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

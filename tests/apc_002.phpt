--TEST--
APC: apcu_add/fetch with objects
--SKIPIF--
<?php require_once(dirname(__FILE__) . '/skipif.inc'); ?>
--INI--
apc.enabled=1
apc.enable_cli=1
--FILE--
<?php

#[AllowDynamicProperties]
class foo { }
$foo = new foo;
var_dump($foo);
apcu_add('foo',$foo);
unset($foo);
$bar = apcu_fetch('foo');
var_dump($bar);
$bar->a = true;
var_dump($bar);

?>
===DONE===
--EXPECTF--
object(foo)#%d (0) {
}
object(foo)#%d (0) {
}
object(foo)#%d (1) {
  ["a"]=>
  bool(true)
}
===DONE===

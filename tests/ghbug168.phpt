--TEST--
Not really gh bug #168
--SKIPIF--
<?php require_once(dirname(__FILE__) . '/skipif.inc'); ?>
--INI--
apc.enabled=1
apc.enable_cli=1
--FILE--
<?php
apcu_add('prop', 'A');

var_dump($prop = apcu_fetch('prop'));

apcu_add('prop', ['B']);

var_dump(apcu_fetch('prop'), $prop);

apcu_add('thing', ['C']);

var_dump(apcu_fetch('prop'), $prop);
?>
--EXPECT--
string(1) "A"
array(1) {
  [0]=>
  string(1) "B"
}
string(1) "A"
array(1) {
  [0]=>
  string(1) "B"
}
string(1) "A"


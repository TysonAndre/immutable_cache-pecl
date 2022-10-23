--TEST--
APC: apcu_add/fetch with strings
--SKIPIF--
<?php require_once(dirname(__FILE__) . '/skipif.inc'); ?>
--INI--
apc.enabled=1
apc.enable_cli=1
--FILE--
<?php
$foo = 'hello world';
var_dump($foo);
apcu_add('foo',$foo);
$bar = apcu_fetch('foo');
var_dump($bar);
$bar = 'nice';
var_dump($bar);

apcu_add('foo\x00bar', $foo);
$bar = apcu_fetch('foo\x00bar');
var_dump($bar);

?>
===DONE===
--EXPECT--
string(11) "hello world"
string(11) "hello world"
string(4) "nice"
string(11) "hello world"
===DONE===

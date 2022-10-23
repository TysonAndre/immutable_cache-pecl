--TEST--
APC: immutable_cache_add/fetch with strings
--SKIPIF--
<?php require_once(dirname(__FILE__) . '/skipif.inc'); ?>
--INI--
apc.enabled=1
apc.enable_cli=1
--FILE--
<?php
$foo = 'hello world';
var_dump($foo);
immutable_cache_add('foo',$foo);
$bar = immutable_cache_fetch('foo');
var_dump($bar);
$bar = 'nice';
var_dump($bar);

immutable_cache_add('foo\x00bar', $foo);
$bar = immutable_cache_fetch('foo\x00bar');
var_dump($bar);

?>
===DONE===
--EXPECT--
string(11) "hello world"
string(11) "hello world"
string(4) "nice"
string(11) "hello world"
===DONE===

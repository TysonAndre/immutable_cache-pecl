--TEST--
APC: immutable_cache_add/fetch with bools
--SKIPIF--
<?php require_once(dirname(__FILE__) . '/skipif.inc'); ?>
--INI--
apc.enabled=1
apc.enable_cli=1
--FILE--
<?php

$foo = false;
var_dump($foo);     /* false */
immutable_cache_add('foo',$foo);
//$success = "some string";

$bar = immutable_cache_fetch('foo', $success);
var_dump($foo);     /* false */
var_dump($bar);     /* false */
var_dump($success); /* true  */

$bar = immutable_cache_fetch('not foo', $success);
var_dump($foo);     /* false */
var_dump($bar);     /* false */
var_dump($success); /* false */

?>
===DONE===
--EXPECT--
bool(false)
bool(false)
bool(false)
bool(true)
bool(false)
bool(false)
bool(false)
===DONE===

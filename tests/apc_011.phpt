--TEST--
APC: immutable_cache_fetch resets array pointers
--SKIPIF--
<?php require_once(dirname(__FILE__) . '/skipif.inc'); ?>
--INI--
immutable_cache.enabled=1
immutable_cache.enable_cli=1
--FILE--
<?php
$items = array('bar', 'baz');

immutable_cache_add('test', $items);

$back = immutable_cache_fetch('test');

var_dump(current($back));
var_dump(current($back));

?>
===DONE===
--EXPECT--
string(3) "bar"
string(3) "bar"
===DONE===

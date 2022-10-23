--TEST--
GH Bug #247: when a NUL char is used as key, immutable_cache_fetch(array) truncates the key
--SKIPIF--
<?php require_once(dirname(__FILE__) . '/skipif.inc'); ?>
--INI--
immutable_cache.enabled=1
immutable_cache.enable_cli=1
--FILE--
<?php
immutable_cache_add(array("a\0b" => 'foo'));
var_dump(immutable_cache_fetch(array("a\0b"))["a\0b"]);
?>
--EXPECT--
string(3) "foo"

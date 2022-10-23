--TEST--
Using empty string as key
--SKIPIF--
<?php require_once(dirname(__FILE__) . '/skipif.inc'); ?>
--INI--
immutable_cache.enabled=1
immutable_cache.enable_cli=1
--FILE--
<?php

var_dump(immutable_cache_add("", 123));
var_dump(immutable_cache_exists(""));
var_dump(immutable_cache_fetch(""));

?>
--EXPECT--
bool(true)
bool(true)
int(123)

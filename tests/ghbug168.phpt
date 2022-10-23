--TEST--
gh bug #168 (no longer applies in immutable)
--SKIPIF--
<?php require_once(dirname(__FILE__) . '/skipif.inc'); ?>
--INI--
immutable_cache.enabled=1
immutable_cache.enable_cli=1
--FILE--
<?php
immutable_cache_add('prop', 'A');

var_dump($prop = immutable_cache_fetch('prop'));

immutable_cache_add('prop', ['B']);

var_dump(immutable_cache_fetch('prop'), $prop);

immutable_cache_add('thing', ['C']);

var_dump(immutable_cache_fetch('prop'), $prop);
?>
--EXPECT--
string(1) "A"
string(1) "A"
string(1) "A"
string(1) "A"
string(1) "A"
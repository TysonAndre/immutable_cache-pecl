--TEST--
APC: immutable_cache_fetch returns same string
--SKIPIF--
<?php require_once(dirname(__FILE__) . '/skipif.inc'); ?>
--INI--
immutable_cache.enabled=1
immutable_cache.enable_cli=1
immutable_cache.serializer=default
--FILE--
<?php

$prefix = 'a';
$orig = "{$prefix}xyz";
debug_zval_dump($orig);
immutable_cache_add('key', $orig);
$v1 = immutable_cache_fetch('key');
$v2 = immutable_cache_fetch('key');
$v2[1] = '?';
echo "orig, v1, v2\n";
debug_zval_dump($orig, $v1, $v2);
?>
--EXPECT--
string(4) "axyz" refcount(2)
orig, v1, v2
string(4) "axyz" refcount(2)
string(4) "axyz" interned
string(4) "a?yz" refcount(2)

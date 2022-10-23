--TEST--
APC should not preserve the IAP
--SKIPIF--
<?php require_once(dirname(__FILE__) . '/skipif.inc'); ?>
--INI--
immutable_cache.enabled=1
immutable_cache.enable_cli=1
immutable_cache.serializer=default
--FILE--
<?php

$array = [1, 2, 3];
next($array);
immutable_cache_add("ary", $array);
$array = immutable_cache_fetch("ary");
var_dump(current($array));

?>
--EXPECT--
int(1)

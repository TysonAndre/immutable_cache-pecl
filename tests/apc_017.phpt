--TEST--
APC should not preserve the IAP
--SKIPIF--
<?php require_once(dirname(__FILE__) . '/skipif.inc'); ?>
--INI--
apc.enabled=1
apc.enable_cli=1
apc.serializer=default
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

--TEST--
The same string is used as the cache key and an array key
--INI--
apc.enabled=1
apc.enable_cli=1
apc.serializer=default
--FILE--
<?php

$key = 'key';
$a = [$key => null];
var_dump(immutable_cache_add($key, $a));
var_dump(immutable_cache_fetch($key));

?>
--EXPECT--
bool(true)
array(1) {
  ["key"]=>
  NULL
}
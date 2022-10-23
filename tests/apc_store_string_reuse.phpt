--TEST--
The same string is used as the cache key and an array key
--INI--
immutable_cache.enabled=1
immutable_cache.enable_cli=1
immutable_cache.serializer=default
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
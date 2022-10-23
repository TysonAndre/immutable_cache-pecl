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
var_dump(apcu_add($key, $a));
var_dump(apcu_fetch($key));

?>
--EXPECT--
bool(true)
array(1) {
  ["key"]=>
  NULL
}
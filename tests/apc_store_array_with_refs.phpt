--TEST--
Store array that references same value twice
--INI--
immutable_cache.enabled=1
immutable_cache.enable_cli=1
immutable_cache.serializer=default
--FILE--
<?php

immutable_cache_add('test', [&$x, &$x]);
var_dump(immutable_cache_fetch('test'));

?>
--EXPECT--
array(2) {
  [0]=>
  &NULL
  [1]=>
  &NULL
}

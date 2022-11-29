--TEST--
The outermost value should always be a value, not a reference
--INI--
immutable_cache.enabled=1
immutable_cache.enable_cli=1
immutable_cache.serializer=default
--SKIPIF--
<?php if (PHP_VERSION_ID >= 80000) die('skip Requires PHP < 8.0.0'); ?>
--FILE--
<?php

/* The output is different for the php serializer, because it does not replicate the
 * cycle involving the top-level value. Instead the cycle is placed one level lower.
 * I believe this is a bug in the php serializer. */

$value = [&$value];
immutable_cache_add(["key" => &$value]);
$result = immutable_cache_fetch("key");
var_dump($result);

?>
--EXPECT--
array(1) {
  [0]=>
  &array(1) {
    [0]=>
    *RECURSION*
  }
}

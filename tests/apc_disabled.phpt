--TEST--
Behavior of functions when immutable_cache is disabled
--INI--
immutable_cache.enable_cli=0
--FILE--
<?php

echo "enabled\n";
var_dump(immutable_cache_enabled());

echo "\ncache/sma_info\n";
var_dump(immutable_cache_cache_info());
var_dump(immutable_cache_sma_info());

echo "\add/exists/fetch/key_info\n";
var_dump(immutable_cache_add("key", "value"));
var_dump(immutable_cache_exists("key"));
var_dump(immutable_cache_fetch("key"));
var_dump(immutable_cache_key_info("key"));

echo "\add/exists/fetch array\n";
var_dump(immutable_cache_add(["key" => "value"]));
var_dump(immutable_cache_exists(["key"]));
var_dump(immutable_cache_fetch(["key"]));

echo "\niterator\n";
try {
    new ImmutableCacheIterator;
} catch (Error $e) {
    echo $e->getMessage(), "\n";
}

?>
--EXPECTF--
enabled
bool(false)

cache/sma_info

Warning: immutable_cache_cache_info(): No immutable_cache info available.  Perhaps immutable_cache is not enabled? Check immutable_cache.enabled in your ini file in %sapc_disabled.php on line 7
bool(false)

Warning: immutable_cache_sma_info(): No immutable_cache SMA info available.  Perhaps immutable_cache is disabled via immutable_cache.enabled? in %sapc_disabled.php on line 8
bool(false)
\add/exists/fetch/key_info
bool(false)
bool(false)
bool(false)
NULL
\add/exists/fetch array
array(1) {
  ["key"]=>
  int(-1)
}
array(0) {
}
array(0) {
}

iterator
APC must be enabled to use ImmutableCacheIterator

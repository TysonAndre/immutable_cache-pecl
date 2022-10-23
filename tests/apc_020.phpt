--TEST--
Test values not expunged, have no expiries
--SKIPIF--
<?php
require_once(__DIR__ . '/skipif.inc');
if (!function_exists('immutable_cache_inc_request_time')) die('skip APC debug build required');
?>
--INI--
apc.enabled=1
apc.enable_cli=1
--FILE--
<?php

immutable_cache_add("no_ttl_unaccessed", 12);
immutable_cache_add("no_ttl_accessed", 24);
immutable_cache_add("ttl", 42, 3);
immutable_cache_add("dummy", "xxx");

immutable_cache_inc_request_time(1);
immutable_cache_fetch("no_ttl_accessed");

immutable_cache_inc_request_time(1);

// Fill the cache
$i = 0;
while (immutable_cache_exists("dummy")) {
    immutable_cache_add("key" . $i, str_repeat('x', 500));
    $i++;
}

var_dump(immutable_cache_fetch("no_ttl_unaccessed"));
var_dump(immutable_cache_fetch("no_ttl_accessed"));
var_dump(immutable_cache_fetch("ttl"));

?>
--EXPECT--
bool(false)
int(24)
int(42)

--TEST--
immutable_cache_inc/dec() TTL parameter
--SKIPIF--
<?php
require_once(__DIR__ . '/skipif.inc');
if (!function_exists('immutable_cache_inc_request_time')) die('skip APC debug build required');
?>
--INI--
immutable_cache.enabled=1
immutable_cache.enable_cli=1
immutable_cache.use_request_time=1
immutable_cache.ttl=0
--FILE--
<?php

$ttl = 1;
immutable_cache_add("a", 0, $ttl);
immutable_cache_add("b", 0, $ttl);

for ($i = 0; $i < 6; $i++) {
    echo "T+$i:\n";
    var_dump(immutable_cache_inc("a"));
    var_dump(immutable_cache_inc("b", 1, $success, $ttl));
    immutable_cache_inc_request_time(1);
}

?>
--EXPECT--
T+0:
int(1)
int(1)
T+1:
int(2)
int(2)
T+2:
int(1)
int(1)
T+3:
int(2)
int(2)
T+4:
int(3)
int(1)
T+5:
int(4)
int(2)

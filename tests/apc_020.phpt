--TEST--
Test values not expunged, have no expiries
--SKIPIF--
<?php
require_once(__DIR__ . '/skipif.inc');
if (!function_exists('apcu_inc_request_time')) die('skip APC debug build required');
?>
--INI--
apc.enabled=1
apc.enable_cli=1
--FILE--
<?php

apcu_add("no_ttl_unaccessed", 12);
apcu_add("no_ttl_accessed", 24);
apcu_add("ttl", 42, 3);
apcu_add("dummy", "xxx");

apcu_inc_request_time(1);
apcu_fetch("no_ttl_accessed");

apcu_inc_request_time(1);

// Fill the cache
$i = 0;
while (apcu_exists("dummy")) {
    apcu_add("key" . $i, str_repeat('x', 500));
    $i++;
}

var_dump(apcu_fetch("no_ttl_unaccessed"));
var_dump(apcu_fetch("no_ttl_accessed"));
var_dump(apcu_fetch("ttl"));

?>
--EXPECT--
bool(false)
int(24)
int(42)

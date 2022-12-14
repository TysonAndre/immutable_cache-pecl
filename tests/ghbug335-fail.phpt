--TEST--
Stampede protection was removed.
--SKIPIF--
<?php
require_once(dirname(__FILE__) . '/skipif.inc');
if (!extension_loaded('pcntl')) {
  die('skip pcntl required');
}
?>
--INI--
immutable_cache.enabled=1
immutable_cache.enable_cli=1
--FILE--
<?php

// Reset slam detection.
immutable_cache_add("foo", "parent");

$pid = pcntl_fork();
if ($pid) {
    // parent
    pcntl_wait($status);
} else {
    // child
    $ret = immutable_cache_add("foo", "child");
    if ($ret === false) {
        echo "Stampede protection was removed\n";
    } else {
        echo "Stampede protection doesn't work\n";
    }
    exit(0);
}

?>
--EXPECT--
Stampede protection was removed

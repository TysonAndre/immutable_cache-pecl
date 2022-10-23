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
apc.enabled=1
apc.enable_cli=1
apc.use_request_time=1
--FILE--
<?php

// Reset slam detection.
apcu_add("foo", "parent");

$pid = pcntl_fork();
if ($pid) {
    // parent
    pcntl_wait($status);
} else {
    // child
    $ret = apcu_add("foo", "child");
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

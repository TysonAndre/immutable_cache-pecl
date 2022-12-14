--TEST--
APC: immutable_cache_add/fetch/add with array of key/value pairs.
--SKIPIF--
<?php require_once(dirname(__FILE__) . '/skipif.inc'); ?>
--INI--
immutable_cache.enabled=1
immutable_cache.enable_cli=1
--FILE--
<?php

$entries = array();
$entries['key1'] = 'value1';
$entries['key2'] = 'value2';
$entries['key3'] = array('value3a','value3b');
$entries['key4'] = 4;

var_dump(immutable_cache_add($entries));
$cached_values = immutable_cache_fetch(array_keys($entries));
var_dump($cached_values);

$cached_values = immutable_cache_fetch(array_keys($entries));
var_dump($cached_values);
var_dump(immutable_cache_add($entries));
$cached_values = immutable_cache_fetch(array_keys($entries));
var_dump($cached_values);

?>
===DONE===
--EXPECT--
array(0) {
}
array(4) {
  ["key1"]=>
  string(6) "value1"
  ["key2"]=>
  string(6) "value2"
  ["key3"]=>
  array(2) {
    [0]=>
    string(7) "value3a"
    [1]=>
    string(7) "value3b"
  }
  ["key4"]=>
  int(4)
}
array(4) {
  ["key1"]=>
  string(6) "value1"
  ["key2"]=>
  string(6) "value2"
  ["key3"]=>
  array(2) {
    [0]=>
    string(7) "value3a"
    [1]=>
    string(7) "value3b"
  }
  ["key4"]=>
  int(4)
}
array(4) {
  ["key1"]=>
  int(-1)
  ["key2"]=>
  int(-1)
  ["key3"]=>
  int(-1)
  ["key4"]=>
  int(-1)
}
array(4) {
  ["key1"]=>
  string(6) "value1"
  ["key2"]=>
  string(6) "value2"
  ["key3"]=>
  array(2) {
    [0]=>
    string(7) "value3a"
    [1]=>
    string(7) "value3b"
  }
  ["key4"]=>
  int(4)
}
===DONE===
--TEST--
APC: immutable_cache_fetch returns same array
--SKIPIF--
<?php require_once(dirname(__FILE__) . '/skipif.inc'); ?>
--INI--
immutable_cache.enabled=1
immutable_cache.enable_cli=1
immutable_cache.serializer=default
--FILE--
<?php

// Arrays containing nan are only identical if the array pointers are identical
$str = 'a';
$orig = [NAN];
immutable_cache_add("x$str", $orig);
$c1 = immutable_cache_fetch('xa');
var_dump($c1);
immutable_cache_add("y$str", $c1);
$c2 = immutable_cache_fetch("y$str");
var_dump($c1 === $c2);
$orig = [NAN, "{$str}xyz", 'a', 'a'];
immutable_cache_add("z$str", $orig);
$v1 = immutable_cache_fetch('za');
$v2 = immutable_cache_fetch('za');
echo "is same array?\n";
var_dump($v1 === $v2);
echo "is shared memory array different from original?\n";
var_dump($orig === $v2);
$v1['axyz'] = new stdClass();
var_dump($v1);
var_dump($v2);
unset($v1, $v2, $orig);
?>
--EXPECT--
array(1) {
  [0]=>
  float(NAN)
}
Already persisted this array
bool(true)
is same array?
bool(true)
is shared memory array different from original?
bool(false)
array(5) {
  [0]=>
  float(NAN)
  [1]=>
  string(4) "axyz"
  [2]=>
  string(1) "a"
  [3]=>
  string(1) "a"
  ["axyz"]=>
  object(stdClass)#1 (0) {
  }
}
array(4) {
  [0]=>
  float(NAN)
  [1]=>
  string(4) "axyz"
  [2]=>
  string(1) "a"
  [3]=>
  string(1) "a"
}

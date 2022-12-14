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

$str = 'a';
$orig = ["{$str}xyz" => NAN];
immutable_cache_add('key', $orig);
$v1 = immutable_cache_fetch('key');
$v2 = immutable_cache_fetch('key');
echo "is same array?\n";
var_dump($v1 === $v2);
echo "is shared memory array different from original?\n";
var_dump($orig === $v2);
$v1['axyz'] = new stdClass();
var_dump($v2);
unset($v1, $v2, $orig);
?>
--EXPECT--
is same array?
bool(true)
is shared memory array different from original?
bool(false)
array(1) {
  ["axyz"]=>
  float(NAN)
}

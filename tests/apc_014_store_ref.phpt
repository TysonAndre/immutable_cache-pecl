--TEST--
APC: store array of references
--SKIPIF--
<?php require_once(dirname(__FILE__) . '/skipif.inc'); ?>
--INI--
immutable_cache.enabled=1
immutable_cache.enable_cli=1
immutable_cache.serializer=php
--FILE--
<?php
$_items = [
	'key1' => 'value1',
	'key2' => 'value2',
];
$items = [];
foreach($_items as $k => $v) {
	$items["prefix_$k"] = &$v;
	unset($v);
}
var_dump(immutable_cache_add($items));
var_dump(immutable_cache_fetch('prefix_key1'));
var_dump($items);
echo "After add\n";
var_dump(immutable_cache_fetch('prefix_key1'));
?>
--EXPECT--
array(0) {
}
string(6) "value1"
array(2) {
  ["prefix_key1"]=>
  string(6) "value1"
  ["prefix_key2"]=>
  string(6) "value2"
}
After add
string(6) "value1"

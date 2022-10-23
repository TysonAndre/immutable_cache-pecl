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
}
var_dump(immutable_cache_add($items));
?>
===DONE===
--EXPECT--
array(0) {
}
===DONE===

--TEST--
APC: APCIterator regex & chunk size & list
--SKIPIF--
<?php require_once(dirname(__FILE__) . '/skipif.inc'); ?>
--INI--
apc.enabled=1
apc.enable_cli=1
--FILE--
<?php
$it = new APCuIterator('/key[0-9]0/', APC_ITER_ALL, 1, APC_LIST_ACTIVE);
for($i = 0; $i < 41; $i++) {
  immutable_cache_add("key$i", "value$i");
}
$vals = [];
foreach($it as $key=>$value) {
  $vals[$key] = $value['key'];
}
ksort($vals);
var_dump($vals);

?>
===DONE===
--EXPECT--
array(4) {
  ["key10"]=>
  string(5) "key10"
  ["key20"]=>
  string(5) "key20"
  ["key30"]=>
  string(5) "key30"
  ["key40"]=>
  string(5) "key40"
}
===DONE===

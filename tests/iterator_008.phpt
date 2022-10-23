--TEST--
APC: APCIterator array
--SKIPIF--
<?php require_once(dirname(__FILE__) . '/skipif.inc'); ?>
--INI--
immutable_cache.enabled=1
immutable_cache.enable_cli=1
--FILE--
<?php
$it = new ImmutableCacheIterator(['key1', 'key7', 'key9']);

for($i = 0; $i < 10; $i++) {
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
array(3) {
  ["key1"]=>
  string(4) "key1"
  ["key7"]=>
  string(4) "key7"
  ["key9"]=>
  string(4) "key9"
}
===DONE===

--TEST--
Basic immutable_cache_sma_info() test
--INI--
immutable_cache.enabled=1
immutable_cache.enable_cli=1
immutable_cache.shm_segments=1
--FILE--
<?php

immutable_cache_add("key", "value");
var_dump(immutable_cache_sma_info(true));
var_dump(immutable_cache_sma_info());

?>
--EXPECTF--
array(3) {
  ["num_seg"]=>
  int(1)
  ["seg_size"]=>
  float(%s)
  ["avail_mem"]=>
  float(%s)
}
array(4) {
  ["num_seg"]=>
  int(1)
  ["seg_size"]=>
  float(%s)
  ["avail_mem"]=>
  float(%s)
  ["block_lists"]=>
  array(1) {
    [0]=>
    array(1) {
      [0]=>
      array(2) {
        ["size"]=>
        int(%d)
        ["offset"]=>
        int(%d)
      }
    }
  }
}

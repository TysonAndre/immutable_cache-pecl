--TEST--
APC: APCIterator formats
--SKIPIF--
<?php require_once(dirname(__FILE__) . '/skipif.inc'); ?>
--INI--
immutable_cache.enabled=1
immutable_cache.enable_cli=1
immutable_cache.user_entries_hint=4096
--FILE--
<?php
$formats = array(
                  IMMUTABLE_CACHE_ITER_KEY,
                  IMMUTABLE_CACHE_ITER_VALUE,
                  IMMUTABLE_CACHE_ITER_NUM_HITS,
                  IMMUTABLE_CACHE_ITER_CTIME,
                  IMMUTABLE_CACHE_ITER_ATIME,
                  IMMUTABLE_CACHE_ITER_MEM_SIZE,
                  IMMUTABLE_CACHE_ITER_NONE,
                  IMMUTABLE_CACHE_ITER_ALL & ~IMMUTABLE_CACHE_ITER_TYPE,
                  IMMUTABLE_CACHE_ITER_KEY | IMMUTABLE_CACHE_ITER_NUM_HITS | IMMUTABLE_CACHE_ITER_MEM_SIZE,
                );

$it_array = array();

foreach ($formats as $idx => $format) {
	$it_array[$idx] = new ImmutableCacheIterator(NULL, $format);
}

for($i = 0; $i < 11; $i++) {
  immutable_cache_add("key$i", "value$i");
}

foreach ($it_array as $idx => $it) {
  print_it($it, $idx);
}

function print_it($it, $idx) {
  echo "IT #$idx\n";
  echo "============================\n";
  foreach ($it as $key=>$value) {
    var_dump($key);
    var_dump($value);
  }
  echo "============================\n\n";
}

?>
===DONE===
--EXPECTF--
IT #0
============================
string(4) "key0"
array(1) {
  ["key"]=>
  string(4) "key0"
}
string(4) "key1"
array(1) {
  ["key"]=>
  string(4) "key1"
}
string(4) "key2"
array(1) {
  ["key"]=>
  string(4) "key2"
}
string(4) "key3"
array(1) {
  ["key"]=>
  string(4) "key3"
}
string(4) "key4"
array(1) {
  ["key"]=>
  string(4) "key4"
}
string(4) "key5"
array(1) {
  ["key"]=>
  string(4) "key5"
}
string(4) "key6"
array(1) {
  ["key"]=>
  string(4) "key6"
}
string(4) "key7"
array(1) {
  ["key"]=>
  string(4) "key7"
}
string(4) "key8"
array(1) {
  ["key"]=>
  string(4) "key8"
}
string(4) "key9"
array(1) {
  ["key"]=>
  string(4) "key9"
}
string(5) "key10"
array(1) {
  ["key"]=>
  string(5) "key10"
}
============================

IT #1
============================
string(4) "key0"
array(1) {
  ["value"]=>
  string(6) "value0"
}
string(4) "key1"
array(1) {
  ["value"]=>
  string(6) "value1"
}
string(4) "key2"
array(1) {
  ["value"]=>
  string(6) "value2"
}
string(4) "key3"
array(1) {
  ["value"]=>
  string(6) "value3"
}
string(4) "key4"
array(1) {
  ["value"]=>
  string(6) "value4"
}
string(4) "key5"
array(1) {
  ["value"]=>
  string(6) "value5"
}
string(4) "key6"
array(1) {
  ["value"]=>
  string(6) "value6"
}
string(4) "key7"
array(1) {
  ["value"]=>
  string(6) "value7"
}
string(4) "key8"
array(1) {
  ["value"]=>
  string(6) "value8"
}
string(4) "key9"
array(1) {
  ["value"]=>
  string(6) "value9"
}
string(5) "key10"
array(1) {
  ["value"]=>
  string(7) "value10"
}
============================

IT #2
============================
string(4) "key0"
array(1) {
  ["num_hits"]=>
  int(0)
}
string(4) "key1"
array(1) {
  ["num_hits"]=>
  int(0)
}
string(4) "key2"
array(1) {
  ["num_hits"]=>
  int(0)
}
string(4) "key3"
array(1) {
  ["num_hits"]=>
  int(0)
}
string(4) "key4"
array(1) {
  ["num_hits"]=>
  int(0)
}
string(4) "key5"
array(1) {
  ["num_hits"]=>
  int(0)
}
string(4) "key6"
array(1) {
  ["num_hits"]=>
  int(0)
}
string(4) "key7"
array(1) {
  ["num_hits"]=>
  int(0)
}
string(4) "key8"
array(1) {
  ["num_hits"]=>
  int(0)
}
string(4) "key9"
array(1) {
  ["num_hits"]=>
  int(0)
}
string(5) "key10"
array(1) {
  ["num_hits"]=>
  int(0)
}
============================

IT #3
============================
string(4) "key0"
array(1) {
  ["creation_time"]=>
  int(%d)
}
string(4) "key1"
array(1) {
  ["creation_time"]=>
  int(%d)
}
string(4) "key2"
array(1) {
  ["creation_time"]=>
  int(%d)
}
string(4) "key3"
array(1) {
  ["creation_time"]=>
  int(%d)
}
string(4) "key4"
array(1) {
  ["creation_time"]=>
  int(%d)
}
string(4) "key5"
array(1) {
  ["creation_time"]=>
  int(%d)
}
string(4) "key6"
array(1) {
  ["creation_time"]=>
  int(%d)
}
string(4) "key7"
array(1) {
  ["creation_time"]=>
  int(%d)
}
string(4) "key8"
array(1) {
  ["creation_time"]=>
  int(%d)
}
string(4) "key9"
array(1) {
  ["creation_time"]=>
  int(%d)
}
string(5) "key10"
array(1) {
  ["creation_time"]=>
  int(%d)
}
============================

IT #4
============================
string(4) "key0"
array(1) {
  ["access_time"]=>
  int(%d)
}
string(4) "key1"
array(1) {
  ["access_time"]=>
  int(%d)
}
string(4) "key2"
array(1) {
  ["access_time"]=>
  int(%d)
}
string(4) "key3"
array(1) {
  ["access_time"]=>
  int(%d)
}
string(4) "key4"
array(1) {
  ["access_time"]=>
  int(%d)
}
string(4) "key5"
array(1) {
  ["access_time"]=>
  int(%d)
}
string(4) "key6"
array(1) {
  ["access_time"]=>
  int(%d)
}
string(4) "key7"
array(1) {
  ["access_time"]=>
  int(%d)
}
string(4) "key8"
array(1) {
  ["access_time"]=>
  int(%d)
}
string(4) "key9"
array(1) {
  ["access_time"]=>
  int(%d)
}
string(5) "key10"
array(1) {
  ["access_time"]=>
  int(%d)
}
============================

IT #5
============================
string(4) "key0"
array(1) {
  ["mem_size"]=>
  int(128)
}
string(4) "key1"
array(1) {
  ["mem_size"]=>
  int(128)
}
string(4) "key2"
array(1) {
  ["mem_size"]=>
  int(128)
}
string(4) "key3"
array(1) {
  ["mem_size"]=>
  int(128)
}
string(4) "key4"
array(1) {
  ["mem_size"]=>
  int(128)
}
string(4) "key5"
array(1) {
  ["mem_size"]=>
  int(128)
}
string(4) "key6"
array(1) {
  ["mem_size"]=>
  int(128)
}
string(4) "key7"
array(1) {
  ["mem_size"]=>
  int(128)
}
string(4) "key8"
array(1) {
  ["mem_size"]=>
  int(128)
}
string(4) "key9"
array(1) {
  ["mem_size"]=>
  int(128)
}
string(5) "key10"
array(1) {
  ["mem_size"]=>
  int(128)
}
============================

IT #6
============================
string(4) "key0"
array(0) {
}
string(4) "key1"
array(0) {
}
string(4) "key2"
array(0) {
}
string(4) "key3"
array(0) {
}
string(4) "key4"
array(0) {
}
string(4) "key5"
array(0) {
}
string(4) "key6"
array(0) {
}
string(4) "key7"
array(0) {
}
string(4) "key8"
array(0) {
}
string(4) "key9"
array(0) {
}
string(5) "key10"
array(0) {
}
============================

IT #7
============================
string(4) "key0"
array(6) {
  ["key"]=>
  string(4) "key0"
  ["value"]=>
  string(6) "value0"
  ["num_hits"]=>
  int(0)
  ["creation_time"]=>
  int(%d)
  ["access_time"]=>
  int(%d)
  ["mem_size"]=>
  int(128)
}
string(4) "key1"
array(6) {
  ["key"]=>
  string(4) "key1"
  ["value"]=>
  string(6) "value1"
  ["num_hits"]=>
  int(0)
  ["creation_time"]=>
  int(%d)
  ["access_time"]=>
  int(%d)
  ["mem_size"]=>
  int(128)
}
string(4) "key2"
array(6) {
  ["key"]=>
  string(4) "key2"
  ["value"]=>
  string(6) "value2"
  ["num_hits"]=>
  int(0)
  ["creation_time"]=>
  int(%d)
  ["access_time"]=>
  int(%d)
  ["mem_size"]=>
  int(128)
}
string(4) "key3"
array(6) {
  ["key"]=>
  string(4) "key3"
  ["value"]=>
  string(6) "value3"
  ["num_hits"]=>
  int(0)
  ["creation_time"]=>
  int(%d)
  ["access_time"]=>
  int(%d)
  ["mem_size"]=>
  int(128)
}
string(4) "key4"
array(6) {
  ["key"]=>
  string(4) "key4"
  ["value"]=>
  string(6) "value4"
  ["num_hits"]=>
  int(0)
  ["creation_time"]=>
  int(%d)
  ["access_time"]=>
  int(%d)
  ["mem_size"]=>
  int(128)
}
string(4) "key5"
array(6) {
  ["key"]=>
  string(4) "key5"
  ["value"]=>
  string(6) "value5"
  ["num_hits"]=>
  int(0)
  ["creation_time"]=>
  int(%d)
  ["access_time"]=>
  int(%d)
  ["mem_size"]=>
  int(128)
}
string(4) "key6"
array(6) {
  ["key"]=>
  string(4) "key6"
  ["value"]=>
  string(6) "value6"
  ["num_hits"]=>
  int(0)
  ["creation_time"]=>
  int(%d)
  ["access_time"]=>
  int(%d)
  ["mem_size"]=>
  int(128)
}
string(4) "key7"
array(6) {
  ["key"]=>
  string(4) "key7"
  ["value"]=>
  string(6) "value7"
  ["num_hits"]=>
  int(0)
  ["creation_time"]=>
  int(%d)
  ["access_time"]=>
  int(%d)
  ["mem_size"]=>
  int(128)
}
string(4) "key8"
array(6) {
  ["key"]=>
  string(4) "key8"
  ["value"]=>
  string(6) "value8"
  ["num_hits"]=>
  int(0)
  ["creation_time"]=>
  int(%d)
  ["access_time"]=>
  int(%d)
  ["mem_size"]=>
  int(128)
}
string(4) "key9"
array(6) {
  ["key"]=>
  string(4) "key9"
  ["value"]=>
  string(6) "value9"
  ["num_hits"]=>
  int(0)
  ["creation_time"]=>
  int(%d)
  ["access_time"]=>
  int(%d)
  ["mem_size"]=>
  int(128)
}
string(5) "key10"
array(6) {
  ["key"]=>
  string(5) "key10"
  ["value"]=>
  string(7) "value10"
  ["num_hits"]=>
  int(0)
  ["creation_time"]=>
  int(%d)
  ["access_time"]=>
  int(%d)
  ["mem_size"]=>
  int(128)
}
============================

IT #8
============================
string(4) "key0"
array(3) {
  ["key"]=>
  string(4) "key0"
  ["num_hits"]=>
  int(0)
  ["mem_size"]=>
  int(128)
}
string(4) "key1"
array(3) {
  ["key"]=>
  string(4) "key1"
  ["num_hits"]=>
  int(0)
  ["mem_size"]=>
  int(128)
}
string(4) "key2"
array(3) {
  ["key"]=>
  string(4) "key2"
  ["num_hits"]=>
  int(0)
  ["mem_size"]=>
  int(128)
}
string(4) "key3"
array(3) {
  ["key"]=>
  string(4) "key3"
  ["num_hits"]=>
  int(0)
  ["mem_size"]=>
  int(128)
}
string(4) "key4"
array(3) {
  ["key"]=>
  string(4) "key4"
  ["num_hits"]=>
  int(0)
  ["mem_size"]=>
  int(128)
}
string(4) "key5"
array(3) {
  ["key"]=>
  string(4) "key5"
  ["num_hits"]=>
  int(0)
  ["mem_size"]=>
  int(128)
}
string(4) "key6"
array(3) {
  ["key"]=>
  string(4) "key6"
  ["num_hits"]=>
  int(0)
  ["mem_size"]=>
  int(128)
}
string(4) "key7"
array(3) {
  ["key"]=>
  string(4) "key7"
  ["num_hits"]=>
  int(0)
  ["mem_size"]=>
  int(128)
}
string(4) "key8"
array(3) {
  ["key"]=>
  string(4) "key8"
  ["num_hits"]=>
  int(0)
  ["mem_size"]=>
  int(128)
}
string(4) "key9"
array(3) {
  ["key"]=>
  string(4) "key9"
  ["num_hits"]=>
  int(0)
  ["mem_size"]=>
  int(128)
}
string(5) "key10"
array(3) {
  ["key"]=>
  string(5) "key10"
  ["num_hits"]=>
  int(0)
  ["mem_size"]=>
  int(128)
}
============================

===DONE===
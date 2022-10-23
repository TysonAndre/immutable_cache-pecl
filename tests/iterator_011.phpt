--TEST--
APCUIterator key() and current() on invalid iterator
--SKIPIF--
<?php require_once(dirname(__FILE__) . '/skipif.inc'); ?>
--INI--
apc.enabled=1
apc.enable_cli=1
--FILE--
<?php

var_dump(immutable_cache_add("key1", "value1"));

$it = new APCUIterator(null, IMMUTABLE_CACHE_ITER_VALUE);
var_dump($it->key());
var_dump($it->current());
$it->next();

try {
    var_dump($it->key());
} catch (Error $e) {
    echo $e->getMessage(), "\n";
}
try {
    var_dump($it->current());
} catch (Error $e) {
    echo $e->getMessage(), "\n";
}

?>
--EXPECT--
bool(true)
string(4) "key1"
array(1) {
  ["value"]=>
  string(6) "value1"
}
Cannot call key() on invalid iterator
Cannot call current() on invalid iterator

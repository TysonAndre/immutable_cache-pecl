--TEST--
APC: APCIterator key invalidated between key() calls (changed for immutable)
--SKIPIF--
<?php require_once(dirname(__FILE__) . '/skipif.inc'); ?>
--INI--
immutable_cache.enabled=1
immutable_cache.enable_cli=1
--FILE--
<?php

var_dump(immutable_cache_add("foo", 0));
$it = new APCuIterator();
$it->rewind();
var_dump($it->key());
echo "add not repeat\n";
var_dump(immutable_cache_add("bar", 0));
var_dump($it->key());

?>
--EXPECT--
bool(true)
string(3) "foo"
add not repeat
bool(true)
string(3) "foo"
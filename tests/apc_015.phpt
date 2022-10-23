--TEST--
Copy failure should not create entry
--SKIPIF--
<?php require_once(dirname(__FILE__) . '/skipif.inc'); ?>
--FILE--
<?php
try {
	immutable_cache_add('thing', function(){});
} catch(Exception $ex) {
}

var_dump(immutable_cache_exists('thing'));
?>
--EXPECT--
bool(false)

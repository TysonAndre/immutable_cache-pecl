--TEST--
APC: ImmutableCacheIterator Subclassing forbidden
--SKIPIF--
<?php require_once(dirname(__FILE__) . '/skipif.inc'); ?>
--INI--
immutable_cache.enabled=1
immutable_cache.enable_cli=1
--FILE--
<?php
class foobar extends ImmutableCacheIterator {
	public function __construct() {}
}
?>
--EXPECTF--
Fatal error: Class foobar%sfinal class%sImmutableCacheIterator%sin %s on line %d
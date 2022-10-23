--TEST--
APC: APCuIterator Subclassing forbidden
--SKIPIF--
<?php require_once(dirname(__FILE__) . '/skipif.inc'); ?>
--INI--
apc.enabled=1
apc.enable_cli=1
--FILE--
<?php
class foobar extends APCuIterator {
	public function __construct() {}
}
?>
--EXPECTF--
Fatal error: Class foobar cannot extend final class APCUIterator in %s on line 2
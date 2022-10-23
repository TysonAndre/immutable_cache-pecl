--TEST--
APC: immutable_cache_add/fetch with objects
--SKIPIF--
<?php
require_once(dirname(__FILE__) . '/skipif.inc');
if (PHP_VERSION_ID < 80100) die("skip For PHP >= 8.1");
?>
--INI--
apc.enabled=1
apc.enable_cli=1
--FILE--
<?php

#[AllowDynamicProperties]
class foo { }
$foo = new foo;
var_dump($foo);
immutable_cache_add('foo',$foo);
unset($foo);
$bar = immutable_cache_fetch('foo');
var_dump($bar);
$bar->a = true;
var_dump($bar);

#[AllowDynamicProperties]
class bar extends foo
{
	public    $pub = 'bar';
	protected $pro = 'bar';
	private   $pri = 'bar'; // we don't see this, we'd need php 5.1 new serialization

	function __construct()
	{
		$this->bar = true;
	}

	function change()
	{
		$this->pri = 'mod';
	}
}

class baz extends bar
{
	private $pri = 'baz';

	function __construct()
	{
		parent::__construct();
		$this->baz = true;
	}
}

$baz = new baz;
var_dump($baz);
$baz->change();
var_dump($baz);
immutable_cache_add('baz', $baz);
unset($baz);
var_dump(immutable_cache_fetch('baz'));

?>
===DONE===
--EXPECTF--
object(foo)#%d (0) {
}
object(foo)#%d (0) {
}
object(foo)#%d (1) {
  ["a"]=>
  bool(true)
}
object(baz)#%d (6) {
  ["pub"]=>
  string(3) "bar"
  ["pro":protected]=>
  string(3) "bar"
  ["pri":"bar":private]=>
  string(3) "bar"
  ["pri":"baz":private]=>
  string(3) "baz"
  ["bar"]=>
  bool(true)
  ["baz"]=>
  bool(true)
}
object(baz)#%d (6) {
  ["pub"]=>
  string(3) "bar"
  ["pro":protected]=>
  string(3) "bar"
  ["pri":"bar":private]=>
  string(3) "mod"
  ["pri":"baz":private]=>
  string(3) "baz"
  ["bar"]=>
  bool(true)
  ["baz"]=>
  bool(true)
}
object(baz)#%d (6) {
  ["pub"]=>
  string(3) "bar"
  ["pro":protected]=>
  string(3) "bar"
  ["pri":"bar":private]=>
  string(3) "mod"
  ["pri":"baz":private]=>
  string(3) "baz"
  ["bar"]=>
  bool(true)
  ["baz"]=>
  bool(true)
}
===DONE===

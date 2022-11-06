--TEST--
APC: GH Bug #185 memory corruption
--SKIPIF--
<?php require_once(dirname(__FILE__) . '/skipif.inc'); ?>
--INI--
immutable_cache.enabled=1
immutable_cache.enable_cli=1
--FILE--
<?php

class MyApc
{
    private $counterName = 'counter';

    public function setCounterName($value)
    {
        $this->counterName = $value;
    }

    public function getCounters($name)
    {
        $rex = '/^' . preg_quote($name) . '\./';
        $counters = array();

        foreach (new \ImmutableCacheIterator($rex) as $counter) {
            $counters[$counter['key']] = $counter['value'];
        }

        return $counters;
    }

    public function add($key, $data)
    {
        $ret =  immutable_cache_add($key, $data);

        if (true !== $ret) {
            throw new \UnexpectedValueException("immutable_cache_store call failed");
        }

        return $ret;
    }
}

$myapc = new MyApc();

var_dump($counterName = uniqid());
var_dump($myapc->setCounterName($counterName));
var_dump($myapc->add($counterName.'.test', 1));
var_dump($results = $myapc->getCounters($counterName));
?>
Done
--EXPECTF--
string(%d) "%s"
NULL
bool(true)
array(1) {
  ["%s.test"]=>
  int(1)
}
Done

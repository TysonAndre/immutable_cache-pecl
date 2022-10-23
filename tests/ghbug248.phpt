--TEST--
GH Bug #248: immutable_cache_fetch may return values causing zend_mm_corruption or segfaults when custom serializer is used
--SKIPIF--
<?php require_once(dirname(__FILE__) . '/skipif.inc'); ?>
--INI--
apc.enabled=1
apc.enable_cli=1
apc.serializer=default
--FILE--
<?php

function build_array() {
    return [
        'params' => 2,
        'construct' => ['a'],
        'x1' => 'y',
        'x2' => 'y',
        'x3' => 'y',
        'x4' => 'y',
        'x5' => 'y',
        'x6' => 0,
    ];
}

class MyClass {
    private $_params;

    public function __construct($params) {
        var_dump($params);
        $this->_params = $params;
        var_dump($params);
        $this->_params['ids'] = [4];
        $this->_params['loadValue'] = 'x';
        unset($this->_params['params']);
    }
}

function setup() {
    immutable_cache_add('mytestkey', build_array());
}

function test_apcu_fetch() {
    // Or store second?
    $value = immutable_cache_fetch('mytestkey');
    echo "Fetching the value initially stored into apcu:\n";
    var_dump($value);
    echo "Done dumping initial fetch\n\n";

    new MyClass($value);
    echo "\$value was passed by value, not reference. After instantiating class, the array \$value gets modified\n";
    var_dump($value);

    echo "\nAnd calling immutable_cache_fetch again, the original data is preserved (8 keys, params=2)\n";
    var_dump(immutable_cache_fetch('mytestkey'));
}

setup();
test_apcu_fetch();
?>
--EXPECT--
Fetching the value initially stored into apcu:
array(8) {
  ["params"]=>
  int(2)
  ["construct"]=>
  array(1) {
    [0]=>
    string(1) "a"
  }
  ["x1"]=>
  string(1) "y"
  ["x2"]=>
  string(1) "y"
  ["x3"]=>
  string(1) "y"
  ["x4"]=>
  string(1) "y"
  ["x5"]=>
  string(1) "y"
  ["x6"]=>
  int(0)
}
Done dumping initial fetch

array(8) {
  ["params"]=>
  int(2)
  ["construct"]=>
  array(1) {
    [0]=>
    string(1) "a"
  }
  ["x1"]=>
  string(1) "y"
  ["x2"]=>
  string(1) "y"
  ["x3"]=>
  string(1) "y"
  ["x4"]=>
  string(1) "y"
  ["x5"]=>
  string(1) "y"
  ["x6"]=>
  int(0)
}
array(8) {
  ["params"]=>
  int(2)
  ["construct"]=>
  array(1) {
    [0]=>
    string(1) "a"
  }
  ["x1"]=>
  string(1) "y"
  ["x2"]=>
  string(1) "y"
  ["x3"]=>
  string(1) "y"
  ["x4"]=>
  string(1) "y"
  ["x5"]=>
  string(1) "y"
  ["x6"]=>
  int(0)
}
$value was passed by value, not reference. After instantiating class, the array $value gets modified
array(8) {
  ["params"]=>
  int(2)
  ["construct"]=>
  array(1) {
    [0]=>
    string(1) "a"
  }
  ["x1"]=>
  string(1) "y"
  ["x2"]=>
  string(1) "y"
  ["x3"]=>
  string(1) "y"
  ["x4"]=>
  string(1) "y"
  ["x5"]=>
  string(1) "y"
  ["x6"]=>
  int(0)
}

And calling immutable_cache_fetch again, the original data is preserved (8 keys, params=2)
array(8) {
  ["params"]=>
  int(2)
  ["construct"]=>
  array(1) {
    [0]=>
    string(1) "a"
  }
  ["x1"]=>
  string(1) "y"
  ["x2"]=>
  string(1) "y"
  ["x3"]=>
  string(1) "y"
  ["x4"]=>
  string(1) "y"
  ["x5"]=>
  string(1) "y"
  ["x6"]=>
  int(0)
}

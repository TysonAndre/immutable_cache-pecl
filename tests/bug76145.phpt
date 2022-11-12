--TEST--
Bug #76145: Data corruption reading from APCu while unserializing
--INI--
immutable_cache.enabled=1
immutable_cache.enable_cli=1
error_reporting=E_ALL&~E_DEPRECATED
--FILE--
<?php

// @phan-suppress-next-line PhanCompatibleSerializeInterfaceDeprecated
class Session implements \Serializable
{
  public $session;
  public function unserialize($serialized) { $this->session = immutable_cache_fetch('session'); }
  public function serialize() { return ''; }
}

// Create array representing a session associated with a user
// account that is enabled but has not been authenticated.
$session = ['user' => ['enabled' => true], 'authenticated' => false];
$session['user']['authenticated'] = &$session['authenticated'];

immutable_cache_add('session', $session);

// After serializing / deserializing, session checks out as authenticated.
print unserialize(serialize(new Session())) -> session['authenticated'] === true ? 'Authenticated.' : 'Not Authenticated.';
echo "\n";
$session = immutable_cache_fetch('session');
var_dump($session);
$session['authenticated'] = true;
var_dump($session);
echo "after modifying session, the original should be unchanged\n";

var_dump(immutable_cache_fetch('session'));

?>
--EXPECT--
Not Authenticated.
array(2) {
  ["user"]=>
  array(2) {
    ["enabled"]=>
    bool(true)
    ["authenticated"]=>
    &bool(false)
  }
  ["authenticated"]=>
  &bool(false)
}
array(2) {
  ["user"]=>
  array(2) {
    ["enabled"]=>
    bool(true)
    ["authenticated"]=>
    &bool(true)
  }
  ["authenticated"]=>
  &bool(true)
}
after modifying session, the original should be unchanged
array(2) {
  ["user"]=>
  array(2) {
    ["enabled"]=>
    bool(true)
    ["authenticated"]=>
    &bool(false)
  }
  ["authenticated"]=>
  &bool(false)
}

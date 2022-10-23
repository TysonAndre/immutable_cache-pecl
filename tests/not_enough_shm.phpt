--TEST--
Error if cache structures cannot be allocated in SHM
--INI--
immutable_cache.enabled=1
immutable_cache.enable_cli=1
immutable_cache.shm_size=1M
immutable_cache.entries_hint=1000000
--FILE--
Irrelevant
--EXPECTF--
%A: Unable to allocate %d bytes of shared memory for cache structures. Either immutable_cache.shm_size is too small or immutable_cache.entries_hint too large in Unknown on line 0

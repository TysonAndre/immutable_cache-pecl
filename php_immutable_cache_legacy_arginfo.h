/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: f2720fb79967c43fc3094b25cc9638a37eafe5bf */

ZEND_BEGIN_ARG_INFO_EX(arginfo_apcu_cache_info, 0, 0, 0)
	ZEND_ARG_INFO(0, limited)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_apcu_key_info, 0, 0, 1)
	ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

#define arginfo_apcu_sma_info arginfo_apcu_cache_info

ZEND_BEGIN_ARG_INFO_EX(arginfo_apcu_enabled, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_apcu_add, 0, 0, 1)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(0, value)
	ZEND_ARG_INFO(0, ttl)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_apcu_fetch, 0, 0, 1)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(1, success)
ZEND_END_ARG_INFO()

#define arginfo_apcu_exists arginfo_apcu_key_info


PHP_APCU_API ZEND_FUNCTION(immutable_cache_cache_info);
PHP_APCU_API ZEND_FUNCTION(immutable_cache_key_info);
PHP_APCU_API ZEND_FUNCTION(immutable_cache_sma_info);
PHP_APCU_API ZEND_FUNCTION(immutable_cache_enabled);
PHP_APCU_API ZEND_FUNCTION(immutable_cache_add);
PHP_APCU_API ZEND_FUNCTION(immutable_cache_fetch);
PHP_APCU_API ZEND_FUNCTION(immutable_cache_exists);


static const zend_function_entry ext_functions[] = {
	ZEND_FE(immutable_cache_cache_info, arginfo_apcu_cache_info)
	ZEND_FE(immutable_cache_key_info, arginfo_apcu_key_info)
	ZEND_FE(immutable_cache_sma_info, arginfo_apcu_sma_info)
	ZEND_FE(immutable_cache_enabled, arginfo_apcu_enabled)
	ZEND_FE(immutable_cache_add, arginfo_apcu_add)
	ZEND_FE(immutable_cache_fetch, arginfo_apcu_fetch)
	ZEND_FE(immutable_cache_exists, arginfo_apcu_exists)
	ZEND_FE_END
};

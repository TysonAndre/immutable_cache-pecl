/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: f2720fb79967c43fc3094b25cc9638a37eafe5bf */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_apcu_cache_info, 0, 0, MAY_BE_ARRAY|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, limited, _IS_BOOL, 0, "false")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_apcu_key_info, 0, 1, IS_ARRAY, 1)
	ZEND_ARG_TYPE_INFO(0, key, IS_STRING, 0)
ZEND_END_ARG_INFO()

#define arginfo_apcu_sma_info arginfo_apcu_cache_info

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_apcu_enabled, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_apcu_add, 0, 1, MAY_BE_ARRAY|MAY_BE_BOOL)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_TYPE_INFO(0, value, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, ttl, IS_LONG, 0, "0")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_apcu_fetch, 0, 1, IS_MIXED, 0)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO_WITH_DEFAULT_VALUE(1, success, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_apcu_exists, 0, 1, MAY_BE_ARRAY|MAY_BE_BOOL)
	ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()


PHP_IMMUTABLE_CACHE_API ZEND_FUNCTION(immutable_cache_cache_info);
PHP_IMMUTABLE_CACHE_API ZEND_FUNCTION(immutable_cache_key_info);
PHP_IMMUTABLE_CACHE_API ZEND_FUNCTION(immutable_cache_sma_info);
PHP_IMMUTABLE_CACHE_API ZEND_FUNCTION(immutable_cache_enabled);
PHP_IMMUTABLE_CACHE_API ZEND_FUNCTION(immutable_cache_add);
PHP_IMMUTABLE_CACHE_API ZEND_FUNCTION(immutable_cache_fetch);
PHP_IMMUTABLE_CACHE_API ZEND_FUNCTION(immutable_cache_exists);


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

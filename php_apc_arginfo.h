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


PHP_APCU_API ZEND_FUNCTION(apcu_cache_info);
PHP_APCU_API ZEND_FUNCTION(apcu_key_info);
PHP_APCU_API ZEND_FUNCTION(apcu_sma_info);
PHP_APCU_API ZEND_FUNCTION(apcu_enabled);
PHP_APCU_API ZEND_FUNCTION(apcu_add);
PHP_APCU_API ZEND_FUNCTION(apcu_fetch);
PHP_APCU_API ZEND_FUNCTION(apcu_exists);


static const zend_function_entry ext_functions[] = {
	ZEND_FE(apcu_cache_info, arginfo_apcu_cache_info)
	ZEND_FE(apcu_key_info, arginfo_apcu_key_info)
	ZEND_FE(apcu_sma_info, arginfo_apcu_sma_info)
	ZEND_FE(apcu_enabled, arginfo_apcu_enabled)
	ZEND_FE(apcu_add, arginfo_apcu_add)
	ZEND_FE(apcu_fetch, arginfo_apcu_fetch)
	ZEND_FE(apcu_exists, arginfo_apcu_exists)
	ZEND_FE_END
};

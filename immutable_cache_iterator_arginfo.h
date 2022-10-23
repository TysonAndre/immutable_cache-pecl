/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 28118db168a04153562a827e7912d17be9134d54 */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_ImmutableCacheIterator___construct, 0, 0, 0)
	ZEND_ARG_INFO_WITH_DEFAULT_VALUE(0, search, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, format, IS_LONG, 0, "IMMUTABLE_CACHE_ITER_ALL")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, chunk_size, IS_LONG, 0, "0")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, list, IS_LONG, 0, "IMMUTABLE_CACHE_LIST_ACTIVE")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ImmutableCacheIterator_rewind, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_ImmutableCacheIterator_next arginfo_class_ImmutableCacheIterator_rewind

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ImmutableCacheIterator_valid, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_ImmutableCacheIterator_key, 0, 0, MAY_BE_STRING|MAY_BE_LONG)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ImmutableCacheIterator_current, 0, 0, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ImmutableCacheIterator_getTotalHits, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_ImmutableCacheIterator_getTotalSize arginfo_class_ImmutableCacheIterator_getTotalHits

#define arginfo_class_ImmutableCacheIterator_getTotalCount arginfo_class_ImmutableCacheIterator_getTotalHits


ZEND_METHOD(ImmutableCacheIterator, __construct);
ZEND_METHOD(ImmutableCacheIterator, rewind);
ZEND_METHOD(ImmutableCacheIterator, next);
ZEND_METHOD(ImmutableCacheIterator, valid);
ZEND_METHOD(ImmutableCacheIterator, key);
ZEND_METHOD(ImmutableCacheIterator, current);
ZEND_METHOD(ImmutableCacheIterator, getTotalHits);
ZEND_METHOD(ImmutableCacheIterator, getTotalSize);
ZEND_METHOD(ImmutableCacheIterator, getTotalCount);


static const zend_function_entry class_ImmutableCacheIterator_methods[] = {
	ZEND_ME(ImmutableCacheIterator, __construct, arginfo_class_ImmutableCacheIterator___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(ImmutableCacheIterator, rewind, arginfo_class_ImmutableCacheIterator_rewind, ZEND_ACC_PUBLIC)
	ZEND_ME(ImmutableCacheIterator, next, arginfo_class_ImmutableCacheIterator_next, ZEND_ACC_PUBLIC)
	ZEND_ME(ImmutableCacheIterator, valid, arginfo_class_ImmutableCacheIterator_valid, ZEND_ACC_PUBLIC)
	ZEND_ME(ImmutableCacheIterator, key, arginfo_class_ImmutableCacheIterator_key, ZEND_ACC_PUBLIC)
	ZEND_ME(ImmutableCacheIterator, current, arginfo_class_ImmutableCacheIterator_current, ZEND_ACC_PUBLIC)
	ZEND_ME(ImmutableCacheIterator, getTotalHits, arginfo_class_ImmutableCacheIterator_getTotalHits, ZEND_ACC_PUBLIC)
	ZEND_ME(ImmutableCacheIterator, getTotalSize, arginfo_class_ImmutableCacheIterator_getTotalSize, ZEND_ACC_PUBLIC)
	ZEND_ME(ImmutableCacheIterator, getTotalCount, arginfo_class_ImmutableCacheIterator_getTotalCount, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

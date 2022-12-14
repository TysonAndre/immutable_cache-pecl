/*
  +----------------------------------------------------------------------+
  | immutable_cache                                                      |
  +----------------------------------------------------------------------+
  | Copyright (c) 2022 Tyson Andre                                       |
  | This is a fork of the APCu module providing fast immutable caching   |
  | functionality. The original APCu license is below.                   |
  +----------------------------------------------------------------------+
  | Authors of immutable_cache patches: Tyson Andre <tandre@php.net>     |
  +----------------------------------------------------------------------+

  +----------------------------------------------------------------------+
  | APCu                                                                 |
  +----------------------------------------------------------------------+
  | Copyright (c) 2018 The PHP Group                                     |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Nikita Popov <nikic@php.net>                                 |
  +----------------------------------------------------------------------+
 */

#include "immutable_cache.h"
#include "immutable_cache_cache.h"

/* TODO Look into whether macros and functionality can be made to work in PHP < 8.0
 * with true immutability. */

#if PHP_VERSION_ID < 70300
# define GC_SET_REFCOUNT(ref, rc) (GC_REFCOUNT(ref) = (rc))
# define GC_ADDREF(ref) GC_REFCOUNT(ref)++
# define GC_ARRAY_HAS_IMMUTABLE_PERSISTENT_FLAGS(ref) ((GC_FLAGS((ref)) & (IS_ARRAY_IMMUTABLE)) == IS_ARRAY_IMMUTABLE)
# define GC_STR_HAS_IMMUTABLE_PERSISTENT_FLAGS(ref) ((GC_FLAGS((ref)) & (IS_STR_INTERNED|IS_STR_PERSISTENT)) == (IS_STR_INTERNED|IS_STR_PERSISTENT))
#else
# define GC_SET_PERSISTENT_TYPE(ref, type) \
	(GC_TYPE_INFO(ref) = type | (GC_PERSISTENT << GC_FLAGS_SHIFT))
# define GC_IMMUTABLE_PERSISTENT_FLAGS (GC_IMMUTABLE | GC_PERSISTENT)
# define GC_ARRAY_HAS_IMMUTABLE_PERSISTENT_FLAGS(ref) ((GC_FLAGS((ref)) & GC_IMMUTABLE_PERSISTENT_FLAGS) == GC_IMMUTABLE_PERSISTENT_FLAGS)
# define GC_STR_HAS_IMMUTABLE_PERSISTENT_FLAGS(ref) ((GC_FLAGS((ref)) & GC_IMMUTABLE_PERSISTENT_FLAGS) == GC_IMMUTABLE_PERSISTENT_FLAGS)
#endif

#if PHP_VERSION_ID < 80000
# define GC_REFERENCE IS_REFERENCE
#endif

/*
 * PERSIST: Copy from request memory to SHM.
 */

typedef struct _immutable_cache_persist_context_t {
	/* Serializer to use */
	immutable_cache_serializer_t *serializer;
	/* Shared memory allocator */
	const immutable_cache_sma_t *sma;
	/* Computed size of the needed SMA allocation */
	size_t size;
	/* Whether or not we may have to memoize refcounted addresses */
	zend_bool memoization_needed;
	/* Whether to serialize the top-level value */
	zend_bool use_serialization;
	/* Serialized object/array string, in case there can only be one */
	unsigned char *serialized_str;
	size_t serialized_str_len;
	/* Whole SMA allocation */
	char *alloc;
	/* Current position in allocation */
	char *alloc_cur;
	/* HashTable storing refcounteds for which the size has already been counted. */
	HashTable already_counted;
	/* HashTable storing already allocated refcounteds. Pointers to refcounteds are stored. */
	HashTable already_allocated;
} immutable_cache_persist_context_t;

#define ADD_SIZE(sz) ctxt->size += ZEND_MM_ALIGNED_SIZE(sz)
#define ADD_SIZE_STR(len) ADD_SIZE(_ZSTR_STRUCT_SIZE(len))

/* Allocate a region of length sz bytes in shared memory */
#define ALLOC(sz) immutable_cache_persist_alloc(ctxt, sz)
/* Copy a value val (of length sz bytes) into shared memory */
#define COPY(val, sz) immutable_cache_persist_alloc_copy(ctxt, val, sz)

static zend_bool immutable_cache_persist_calc_zval(immutable_cache_persist_context_t *ctxt, const zval *zv);
static void immutable_cache_persist_copy_zval_impl(immutable_cache_persist_context_t *ctxt, zval *zv);

/* Used to reduce hash collisions when using pointers in hash tables. (#175) */
static zend_always_inline zend_ulong immutable_cache_shr3(zend_ulong index) {
	return (index >> 3) | (index << (SIZEOF_ZEND_LONG * 8 - 3));
}

static zend_always_inline zend_ulong immutable_cache_pointer_to_hash_key(const zval *ptr) {
	return immutable_cache_shr3((zend_ulong)(uintptr_t)ptr);
}

static inline void immutable_cache_persist_copy_zval(immutable_cache_persist_context_t *ctxt, zval *zv) {
	/* No data apart from the zval itself */
	if (Z_TYPE_P(zv) < IS_STRING) {
		return;
	}

	immutable_cache_persist_copy_zval_impl(ctxt, zv);
}

void immutable_cache_persist_init_context(immutable_cache_persist_context_t *ctxt, immutable_cache_serializer_t *serializer, const immutable_cache_sma_t *sma) {
	ctxt->serializer = serializer;
	ctxt->sma = sma;
	ctxt->size = 0;
	ctxt->memoization_needed = 0;
	ctxt->use_serialization = 0;
	ctxt->serialized_str = NULL;
	ctxt->serialized_str_len = 0;
	ctxt->alloc = NULL;
	ctxt->alloc_cur = NULL;
}

void immutable_cache_persist_destroy_context(immutable_cache_persist_context_t *ctxt) {
	if (ctxt->memoization_needed) {
		zend_hash_destroy(&ctxt->already_counted);
		zend_hash_destroy(&ctxt->already_allocated);
	}
	if (ctxt->serialized_str) {
		efree(ctxt->serialized_str);
	}
}

static zend_bool immutable_cache_persist_calc_memoize(immutable_cache_persist_context_t *ctxt, const void *ptr) {
	zval tmp;
	if (!ctxt->memoization_needed) {
		return 0;
	}

	if (zend_hash_index_exists(&ctxt->already_counted, immutable_cache_pointer_to_hash_key(ptr))) {
		return 1;
	}

	ZVAL_NULL(&tmp);
	zend_hash_index_add_new(&ctxt->already_counted, immutable_cache_pointer_to_hash_key(ptr), &tmp);
	return 0;
}

static void immutable_cache_persist_calc_str(immutable_cache_persist_context_t *ctxt, const zend_string *str)
{
	if (immutable_cache_sma_contains_pointer(ctxt->sma, str)) {
		return;
	}
	if (immutable_cache_persist_calc_memoize(ctxt, str)) {
		return;
	}
	ADD_SIZE_STR(ZSTR_LEN(str));
}

static zend_bool immutable_cache_persist_calc_ht(immutable_cache_persist_context_t *ctxt, const HashTable *ht) {
	uint32_t idx;

	ADD_SIZE(sizeof(HashTable));
	if (ht->nNumUsed == 0) {
		/* TODO reuse an immutable empty array in shared memory */
		return 1;
	}

	/* TODO Too sparse hashtables could be compacted here */
#if PHP_VERSION_ID >= 80200
	if (HT_IS_PACKED(ht)) {
		ADD_SIZE(HT_PACKED_USED_SIZE(ht));
		for (idx = 0; idx < ht->nNumUsed; idx++) {
			zval *val = ht->arPacked + idx;
			ZEND_ASSERT(Z_TYPE_P(val) != IS_INDIRECT && "INDIRECT in packed array?");
			if (!immutable_cache_persist_calc_zval(ctxt, val)) {
				return 0;
			}
		}
	} else
#endif
	{
		ADD_SIZE(HT_USED_SIZE(ht));
		for (idx = 0; idx < ht->nNumUsed; idx++) {
			Bucket *p = ht->arData + idx;
			if (Z_TYPE(p->val) == IS_UNDEF) continue;

			/* This can only happen if $GLOBALS is placed in the cache.
			 * Don't bother with this edge-case, fall back to serialization. */
			if (Z_TYPE(p->val) == IS_INDIRECT) {
				ctxt->use_serialization = 1;
				return 0;
			}

			if (p->key) {
				immutable_cache_persist_calc_str(ctxt, p->key);
			}
			if (!immutable_cache_persist_calc_zval(ctxt, &p->val)) {
				return 0;
			}
		}
	}

	return 1;
}

static zend_bool immutable_cache_persist_calc_serialize(immutable_cache_persist_context_t *ctxt, const zval *zv) {
	unsigned char *buf = NULL;
	size_t buf_len = 0;

	immutable_cache_serialize_t serialize = IMMUTABLE_CACHE_SERIALIZER_NAME(php);
	void *config = NULL;
	if (ctxt->serializer) {
		serialize = ctxt->serializer->serialize;
		config = ctxt->serializer->config;
	}

	if (!serialize(&buf, &buf_len, zv, config)) {
		return 0;
	}

	/* We only ever serialize the top-level value, memoization cannot be needed */
	ZEND_ASSERT(!ctxt->memoization_needed);
	ctxt->serialized_str = buf;
	ctxt->serialized_str_len = buf_len;

	ADD_SIZE_STR(buf_len);
	return 1;
}

/* Returns true if it is safe to store this zval in immutable_cache */
static zend_bool immutable_cache_persist_calc_zval(immutable_cache_persist_context_t *ctxt, const zval *zv) {
	/* TODO: Check if the value being referred to is already an immutable cache entry? */

	if (Z_TYPE_P(zv) < IS_STRING) {
		/* No data apart from the zval itself */
		return 1;
	}

	if (ctxt->use_serialization) {
		return immutable_cache_persist_calc_serialize(ctxt, zv);
	}
	if (immutable_cache_sma_contains_pointer(ctxt->sma, Z_PTR_P(zv))) {
		ZEND_ASSERT(Z_TYPE_P(zv) == IS_STRING || Z_TYPE_P(zv) == IS_ARRAY);
		return 1;
	}

	if (immutable_cache_persist_calc_memoize(ctxt, Z_COUNTED_P(zv))) {
		return 1;
	}

	switch (Z_TYPE_P(zv)) {
		case IS_STRING:
			ADD_SIZE_STR(Z_STRLEN_P(zv));
			return 1;
		case IS_ARRAY:
			return immutable_cache_persist_calc_ht(ctxt, Z_ARRVAL_P(zv));
		case IS_REFERENCE:
			/* TODO support creating copies of references mixed with immutable arrays.
			 * Not yet tested, so fall through and use a serializer
			ADD_SIZE(sizeof(zend_reference));
			return immutable_cache_persist_calc_zval(ctxt, Z_REFVAL_P(zv));
			*/
			/* FALLTHROUGH */
		case IS_OBJECT:
			ctxt->use_serialization = 1;
			return 0;
		case IS_RESOURCE:
			immutable_cache_warning("Cannot store resources in immutable_cache cache");
			return 0;
		EMPTY_SWITCH_DEFAULT_CASE()
	}
}

static zend_bool immutable_cache_persist_calc(immutable_cache_persist_context_t *ctxt, const immutable_cache_cache_entry_t *entry) {
	ADD_SIZE(sizeof(immutable_cache_cache_entry_t));
	immutable_cache_persist_calc_str(ctxt, entry->key);
	return immutable_cache_persist_calc_zval(ctxt, &entry->val);
}

/* Used when persisting to get any immutable_cache shared memory copy of ptr that was already created.
 * If ptr is already in immutable_cache (e.g. in a previously allocated entry), returns ptr.
 * If ptr is found as a key in the already_allocated hash map, then it returns that. */
static inline void *immutable_cache_persist_get_already_allocated(immutable_cache_persist_context_t *ctxt, void *ptr) {
	if (immutable_cache_sma_contains_pointer(ctxt->sma, ptr)) {
		// fprintf(stderr, "%s: returning already allocated %p\n", __func__, ptr);
		return ptr;
	}
	if (ctxt->memoization_needed) {
		return zend_hash_index_find_ptr(&ctxt->already_allocated, immutable_cache_pointer_to_hash_key(ptr));
	}
	return NULL;
}

static inline void immutable_cache_persist_add_already_allocated(
		immutable_cache_persist_context_t *ctxt, const void *old_ptr, void *new_ptr) {
	if (ctxt->memoization_needed) {
		zend_hash_index_add_new_ptr(&ctxt->already_allocated, immutable_cache_pointer_to_hash_key(old_ptr), new_ptr);
	}
}

static inline void *immutable_cache_persist_alloc(immutable_cache_persist_context_t *ctxt, size_t size) {
	void *ptr = ctxt->alloc_cur;
	ctxt->alloc_cur += ZEND_MM_ALIGNED_SIZE(size);
	ZEND_ASSERT(ctxt->alloc_cur <= ctxt->alloc + ctxt->size);
	return ptr;
}

static inline void *immutable_cache_persist_alloc_copy(
		immutable_cache_persist_context_t *ctxt, const void *val, size_t size) {
    /* TODO is it safe to reuse this if it's already in shared memory */
	void *ptr = immutable_cache_persist_alloc(ctxt, size);
	memcpy(ptr, val, size);
	return ptr;
}

/* Creates an immutable zend_string in shared memory with the hash precomputed */
static zend_string *immutable_cache_persist_copy_cstr(
		immutable_cache_persist_context_t *ctxt, const char *orig_buf, size_t buf_len, zend_ulong hash) {
	zend_string *str = ALLOC(_ZSTR_STRUCT_SIZE(buf_len));

	GC_SET_REFCOUNT(str, 2);
#if PHP_VERSION_ID >= 70300
	GC_TYPE_INFO(str) = IS_STRING | (GC_IMMUTABLE_PERSISTENT_FLAGS << GC_FLAGS_SHIFT);
#else
	GC_TYPE_INFO(str) = IS_STRING | ((IS_STR_INTERNED|IS_STR_PERSISTENT) << GC_FLAGS_SHIFT);
#endif

	ZSTR_H(str) = hash;
	ZSTR_LEN(str) = buf_len;
	memcpy(ZSTR_VAL(str), orig_buf, buf_len);
	ZSTR_VAL(str)[buf_len] = '\0';
	zend_string_hash_val(str);

	return str;
}

static inline zend_string *immutable_cache_persist_copy_zstr(
		immutable_cache_persist_context_t *ctxt, zend_string *orig_str) {
	zend_string *str = immutable_cache_persist_copy_cstr(
		ctxt, ZSTR_VAL(orig_str), ZSTR_LEN(orig_str), ZSTR_HASH(orig_str));
	immutable_cache_persist_add_already_allocated(ctxt, orig_str, str);
	return str;
}

/* If the orig_str is already immutable in shared memory, then returns that.
 * If an entry for this string was already added to shared memory for the entry being created, return that.
 * Otherwise, create a new immutable string in shared memory and add it to the already allocated strings */
static inline zend_string *immutable_cache_persist_get_or_copy_zstr(
		immutable_cache_persist_context_t *ctxt, zend_string *orig_str) {
	zend_string *str = immutable_cache_persist_get_already_allocated(ctxt, orig_str);
	if (str) { return str; }

	str = immutable_cache_persist_copy_cstr(
		ctxt, ZSTR_VAL(orig_str), ZSTR_LEN(orig_str), ZSTR_H(orig_str));
	/* TODO: Also create an entry for the string value? */
	immutable_cache_persist_add_already_allocated(ctxt, orig_str, str);
	return str;
}

/* Allocates a persistent copy of the PHP reference group in shared memory, setting a reference count of 1 */
/*
static zend_reference *immutable_cache_persist_copy_ref(
		immutable_cache_persist_context_t *ctxt, const zend_reference *orig_ref) {
	zend_reference *ref = ALLOC(sizeof(zend_reference));
	immutable_cache_persist_add_already_allocated(ctxt, orig_ref, ref);

	GC_SET_REFCOUNT(ref, 1);
	GC_SET_PERSISTENT_TYPE(ref, GC_REFERENCE);
#if PHP_VERSION_ID >= 70400
	ref->sources.ptr = NULL;
#endif

	ZVAL_COPY_VALUE(&ref->val, &orig_ref->val);
	immutable_cache_persist_copy_zval(ctxt, &ref->val);

	return ref;
}
*/

static const uint32_t uninitialized_bucket[-HT_MIN_MASK] = {HT_INVALID_IDX, HT_INVALID_IDX};

static zend_array *immutable_cache_persist_copy_ht(immutable_cache_persist_context_t *ctxt, const HashTable *orig_ht) {
	HashTable *ht = COPY(orig_ht, sizeof(HashTable));
	uint32_t idx;
	immutable_cache_persist_add_already_allocated(ctxt, orig_ht, ht);

	GC_SET_REFCOUNT(ht, 2);
#if PHP_VERSION_ID >= 70300
	GC_TYPE_INFO(ht) = IS_ARRAY | (GC_IMMUTABLE_PERSISTENT_FLAGS << GC_FLAGS_SHIFT);
#else
	GC_TYPE_INFO(ht) = IS_ARRAY | (IS_ARRAY_IMMUTABLE << GC_FLAGS_SHIFT);
#endif

	/* Immutable arrays from opcache may lack a dtor and the apply protection flag. */
	/* NOTE: immutable arrays immutable_cache should not have elements freed by design. This won't get called. */
	ht->pDestructor = NULL; // ht->pDestructor = ZVAL_PTR_DTOR;

#if PHP_VERSION_ID < 70300
	/* Unlike APCu, this is immutable and does not need protection */
	ht->u.flags &= ~HASH_FLAG_APPLY_PROTECTION;
#endif


	/* NOTE: Strings are always static keys here for serializer=default (immutable instead of refcounted), unlike APCu. */
	ht->u.flags |= HASH_FLAG_STATIC_KEYS;
	if (ht->nNumUsed == 0) {
		/* TODO save memory (44 bytes per array padded to alignment) by storing a copy of the empty zend_array at a fixed position on startup */
#if PHP_VERSION_ID >= 70400
		ht->u.flags = HASH_FLAG_UNINITIALIZED;
#else
		ht->u.flags &= ~(HASH_FLAG_INITIALIZED|HASH_FLAG_PACKED);
#endif
		ht->nNextFreeElement = 0;
		ht->nTableMask = HT_MIN_MASK;
		HT_SET_DATA_ADDR(ht, &uninitialized_bucket);
		return ht;
	}

	ht->nNextFreeElement = 0;
	ht->nInternalPointer = HT_INVALID_IDX;
	const uint32_t nNumUsed = ht->nNumUsed;
#if PHP_VERSION_ID >= 80200
	if (HT_IS_PACKED(ht)) {
		HT_SET_DATA_ADDR(ht, COPY(HT_GET_DATA_ADDR(ht), HT_PACKED_USED_SIZE(ht)));
		zval *const arPacked = ht->arPacked;
		for (idx = 0; idx < nNumUsed; idx++) {
			zval *val = arPacked + idx;
			if (Z_TYPE_P(val) == IS_UNDEF) continue;

			if (ht->nInternalPointer == HT_INVALID_IDX) {
				ht->nInternalPointer = idx;
			}

			if ((zend_long) idx >= (zend_long) ht->nNextFreeElement) {
				ht->nNextFreeElement = idx + 1;
			}

			immutable_cache_persist_copy_zval(ctxt, val);
		}
	} else
#endif
	{
		/* NOTE: The extendsions opcache, APCu, and immutable_cache
		 * only need to allocate memory for the buckets that are used in shared memory */
		HT_SET_DATA_ADDR(ht, COPY(HT_GET_DATA_ADDR(ht), HT_USED_SIZE(ht)));
		Bucket *const arData = ht->arData;
		for (idx = 0; idx < nNumUsed; idx++) {
			Bucket *p = arData + idx;
			if (Z_TYPE(p->val) == IS_UNDEF) continue;

			if (ht->nInternalPointer == HT_INVALID_IDX) {
				ht->nInternalPointer = idx;
			}

			if (p->key) {
				p->key = immutable_cache_persist_get_or_copy_zstr(ctxt, p->key);
			} else if ((zend_long) p->h >= (zend_long) ht->nNextFreeElement) {
				ht->nNextFreeElement = p->h + 1;
			}

			immutable_cache_persist_copy_zval(ctxt, &p->val);
		}
	}

	return ht;
}

static void immutable_cache_persist_copy_serialize(
		immutable_cache_persist_context_t *ctxt, zval *zv) {
	zend_string *str;
	zend_uchar orig_type = Z_TYPE_P(zv);
	ZEND_ASSERT(orig_type == IS_ARRAY || orig_type == IS_OBJECT);

	ZEND_ASSERT(!ctxt->memoization_needed);
	ZEND_ASSERT(ctxt->serialized_str);
	str = immutable_cache_persist_copy_cstr(ctxt,
		(char *) ctxt->serialized_str, ctxt->serialized_str_len, 0);

	/* Store as PTR type to distinguish from other strings */
	ZVAL_PTR(zv, str);
}

/* Persist a copy of the zval into the shared memory cache at zval zv. */
static void immutable_cache_persist_copy_zval_impl(immutable_cache_persist_context_t *ctxt, zval *zv) {
	void *ptr;

	if (ctxt->use_serialization) {
		immutable_cache_persist_copy_serialize(ctxt, zv);
		return;
	}

	/* Check if the pointer is already an immutable part of immutable_cache shared memory,
	 * or is found in the hash map */
	ptr = immutable_cache_persist_get_already_allocated(ctxt, Z_COUNTED_P(zv));
	switch (Z_TYPE_P(zv)) {
		case IS_STRING:
			if (!ptr) ptr = immutable_cache_persist_get_or_copy_zstr(ctxt, Z_STR_P(zv));
			ZVAL_STR(zv, ptr);
			return;
		case IS_ARRAY:
			if (!ptr) ptr = immutable_cache_persist_copy_ht(ctxt, Z_ARRVAL_P(zv));
			ZVAL_ARR(zv, ptr);
			/* make immutable array. Based on opcache's zend_persist_zval. */
			Z_TYPE_FLAGS_P(zv) = 0;
			return;
		/* TODO handle unserializing references, test that modifying references doesn't change the original. Add flags to array to indicate the array needs to be copied? */
		case IS_REFERENCE:
			/* This did not put a reference into shared memory */
			zend_error_noreturn(E_CORE_ERROR, "immutable_cache serializer somehow called with a reference instead of a string serialization function");
			/*
			if (!ptr) ptr = immutable_cache_persist_copy_ref(ctxt, Z_REF_P(zv));
			ZVAL_REF(zv, ptr);
			*/
			return;
		EMPTY_SWITCH_DEFAULT_CASE()
	}
}

static immutable_cache_cache_entry_t *immutable_cache_persist_copy(
		immutable_cache_persist_context_t *ctxt, const immutable_cache_cache_entry_t *orig_entry) {
	immutable_cache_cache_entry_t *entry = COPY(orig_entry, sizeof(immutable_cache_cache_entry_t));
	/* entry->key may have been fetched from shared memory by a previous call to immutable_cache_fetch() */
	entry->key = immutable_cache_persist_get_or_copy_zstr(ctxt, entry->key);
	immutable_cache_persist_copy_zval(ctxt, &entry->val);
	return entry;
}

immutable_cache_cache_entry_t *immutable_cache_persist(
		immutable_cache_sma_t *sma, immutable_cache_serializer_t *serializer, const immutable_cache_cache_entry_t *orig_entry) {
	immutable_cache_persist_context_t ctxt;
	immutable_cache_cache_entry_t *entry;

	immutable_cache_persist_init_context(&ctxt, serializer, sma);

	/* The top-level value should never be a reference */
	ZEND_ASSERT(Z_TYPE(orig_entry->val) != IS_REFERENCE);

	/* If we're serializing an array using the default serializer, we will have
	 * to keep track of potentially repeated refcounted structures. */
	if (!serializer && Z_TYPE(orig_entry->val) == IS_ARRAY) {
		ctxt.memoization_needed = 1;
		zend_hash_init(&ctxt.already_counted, 0, NULL, NULL, 0);
		zend_hash_init(&ctxt.already_allocated, 0, NULL, NULL, 0);
	}

	/* Objects are always serialized, and arrays when a serializer is set.
	 * Other cases are detected during immutable_cache_persist_calc(). */
	if (Z_TYPE(orig_entry->val) == IS_OBJECT
			|| (serializer && Z_TYPE(orig_entry->val) == IS_ARRAY)) {
		ctxt.use_serialization = 1;
	}

	if (!immutable_cache_persist_calc(&ctxt, orig_entry)) {
		if (!ctxt.use_serialization) {
			immutable_cache_persist_destroy_context(&ctxt);
			return NULL;
		}

		/* Try again with serialization */
		immutable_cache_persist_destroy_context(&ctxt);
		immutable_cache_persist_init_context(&ctxt, serializer, sma);
		ctxt.use_serialization = 1;
		if (!immutable_cache_persist_calc(&ctxt, orig_entry)) {
			immutable_cache_persist_destroy_context(&ctxt);
			return NULL;
		}
	}

	ctxt.alloc = ctxt.alloc_cur = immutable_cache_sma_malloc(sma, ctxt.size);
	if (!ctxt.alloc) {
		immutable_cache_persist_destroy_context(&ctxt);
		return NULL;
	}

	IMMUTABLE_CACHE_SMA_UNPROTECT_MEMORY(sma);
	entry = immutable_cache_persist_copy(&ctxt, orig_entry);
	// fprintf(stderr, "alloc_size=%d size=%d\n", (int)(ctxt.alloc_cur-ctxt.alloc), (int)ctxt.size);
	ZEND_ASSERT(ctxt.alloc_cur == ctxt.alloc + ctxt.size);

	entry->mem_size = ctxt.size;
	IMMUTABLE_CACHE_SMA_PROTECT_MEMORY(sma);

	immutable_cache_persist_destroy_context(&ctxt);
	return entry;
}

/*
 * UNPERSIST: Copy from SHM to request memory.
 */

typedef struct _immutable_cache_unpersist_context_t {
	/* Whether we need to memoize already copied refcounteds. */
	zend_bool memoization_needed;
	/* HashTable storing already copied refcounteds. */
	HashTable already_copied;
} immutable_cache_unpersist_context_t;

static void immutable_cache_unpersist_zval_impl(immutable_cache_unpersist_context_t *ctxt, zval *zv);

static inline void immutable_cache_unpersist_zval(immutable_cache_unpersist_context_t *ctxt, zval *zv) {
	/* No data apart from the zval itself */
	if (Z_TYPE_P(zv) < IS_STRING) {
		return;
	}

	immutable_cache_unpersist_zval_impl(ctxt, zv);
}

/* Unserialize the value from string str into dst.
 * Returns 1 if successful.
 * Sets dst to PHP NULL and returns 0 if unsuccessful */
static zend_bool immutable_cache_unpersist_serialized(
		zval *dst, zend_string *str, immutable_cache_serializer_t *serializer) {
	immutable_cache_unserialize_t unserialize = IMMUTABLE_CACHE_UNSERIALIZER_NAME(php);
	void *config = NULL;

	if (serializer) {
		unserialize = serializer->unserialize;
		config = serializer->config;
	}

	if (unserialize(dst, (unsigned char *) ZSTR_VAL(str), ZSTR_LEN(str), config)) {
		return 1;
	}

	ZVAL_NULL(dst);
	return 0;
}

/*
 * Returns the pointer if one was already copied from shared memory into emalloc
 */
static inline void *immutable_cache_unpersist_get_already_copied(immutable_cache_unpersist_context_t *ctxt, void *ptr) {
	if (ctxt->memoization_needed) {
		return zend_hash_index_find_ptr(&ctxt->already_copied, immutable_cache_pointer_to_hash_key(ptr));
	}
	return NULL;
}

static inline void immutable_cache_unpersist_add_already_copied(
		immutable_cache_unpersist_context_t *ctxt, const void *old_ptr, void *new_ptr) {
	if (ctxt->memoization_needed) {
		zend_hash_index_add_new_ptr(&ctxt->already_copied, immutable_cache_pointer_to_hash_key(old_ptr), new_ptr);
	}
}

static zend_string *immutable_cache_unpersist_zstr(immutable_cache_unpersist_context_t *ctxt, const zend_string *orig_str) {
	/* TODO: Mark strings as persistent and immutable, and avoid allocating a new string entirely */
	// TODO: Implement immutable_cache_is_already_cached_for_unpersist_context, add fields to it
	// ZEND_ASSERT(immutable_cache_is_already_cached_for_unpersist_context(ctxt, orig_str));
	ZEND_ASSERT(GC_STR_HAS_IMMUTABLE_PERSISTENT_FLAGS(orig_str));
	return (zend_string *)orig_str;
}

/*
static zend_reference *immutable_cache_unpersist_ref(
		immutable_cache_unpersist_context_t *ctxt, const zend_reference *orig_ref) {
	zend_reference *ref = emalloc(sizeof(zend_reference));
	immutable_cache_unpersist_add_already_copied(ctxt, orig_ref, ref);

	GC_SET_REFCOUNT(ref, 1);
	GC_TYPE_INFO(ref) = GC_REFERENCE;
#if PHP_VERSION_ID >= 70400
	ref->sources.ptr = NULL;
#endif

	ZVAL_COPY_VALUE(&ref->val, &orig_ref->val);
	immutable_cache_unpersist_zval(ctxt, &ref->val);
	return ref;
}
*/

static zend_array *immutable_cache_unpersist_ht(
		immutable_cache_unpersist_context_t *ctxt, const HashTable *orig_ht) {
	ZEND_ASSERT(GC_ARRAY_HAS_IMMUTABLE_PERSISTENT_FLAGS(orig_ht));
	return (HashTable *)orig_ht;
	/* Original implementation for copying the hash table. TODO: Restore when ready to test mixes of references and immutables in mutable arrays */
#if 0
	HashTable *ht = emalloc(sizeof(HashTable));

	immutable_cache_unpersist_add_already_copied(ctxt, orig_ht, ht);
	memcpy(ht, orig_ht, sizeof(HashTable));
	GC_TYPE_INFO(ht) = GC_ARRAY;
#if PHP_VERSION_ID >= 70300
	/* Caller used ZVAL_EMPTY_ARRAY and set different zval flags instead */
	ZEND_ASSERT(ht->nNumOfElements > 0 && ht->nNumUsed > 0);
#else
	if (ht->nNumUsed == 0) {
		HT_SET_DATA_ADDR(ht, &uninitialized_bucket);
		return ht;
	}
#endif

	HT_SET_DATA_ADDR(ht, emalloc(immutable_cache_compute_ht_data_size(ht)));
	memcpy(HT_GET_DATA_ADDR(ht), HT_GET_DATA_ADDR(orig_ht), HT_HASH_SIZE(ht->nTableMask));

#if PHP_VERSION_ID >= 80200
	if (HT_IS_PACKED(ht)) {
		zval *p = ht->arPacked, *q = orig_ht->arPacked, *p_end = p + ht->nNumUsed;
		for (; p < p_end; p++, q++) {
			*p = *q;
			immutable_cache_unpersist_zval(ctxt, p);
		}
	} else
#endif
	if (ht->u.flags & HASH_FLAG_STATIC_KEYS) {
		Bucket *p = ht->arData, *q = orig_ht->arData, *p_end = p + ht->nNumUsed;
		for (; p < p_end; p++, q++) {
			/* No need to check for UNDEF, as unpersist_zval can be safely called on UNDEF */
			*p = *q;
			immutable_cache_unpersist_zval(ctxt, &p->val);
		}
	} else {
		Bucket *p = ht->arData, *q = orig_ht->arData, *p_end = p + ht->nNumUsed;
		for (; p < p_end; p++, q++) {
			if (Z_TYPE(q->val) == IS_UNDEF) {
				ZVAL_UNDEF(&p->val);
				continue;
			}

			p->val = q->val;
			p->h = q->h;
			if (q->key) {
				p->key = zend_string_dup(q->key, 0);
			} else {
				p->key = NULL;
			}
			immutable_cache_unpersist_zval(ctxt, &p->val);
		}
	}

	return ht;
#endif /* End #if 0 commenting out code */
}

static void immutable_cache_unpersist_zval_impl(immutable_cache_unpersist_context_t *ctxt, zval *zv) {
	void *ptr = immutable_cache_unpersist_get_already_copied(ctxt, Z_COUNTED_P(zv));
	if (ptr) {
		Z_COUNTED_P(zv) = ptr;
		Z_ADDREF_P(zv);
		return;
	}

	switch (Z_TYPE_P(zv)) {
		case IS_STRING:
			ZVAL_INTERNED_STR(zv, immutable_cache_unpersist_zstr(ctxt, Z_STR_P(zv)));
			return;
		case IS_REFERENCE:
			zend_error_noreturn(E_CORE_ERROR, "immutable_cache_unpersist_zval_impl: Found a reference in shared memory. Is shared memory corrupt?");
			/* Z_REF_P(zv) = immutable_cache_unpersist_ref(ctxt, Z_REF_P(zv)); */
			return;
		case IS_ARRAY:
#if PHP_VERSION_ID >= 70300
			if (Z_ARR_P(zv)->nNumOfElements == 0) {
				ZVAL_EMPTY_ARRAY(zv); /* #323 */
				return;
			}
#endif
			Z_ARR_P(zv) = immutable_cache_unpersist_ht(ctxt, Z_ARR_P(zv));
			return;
		default:
			ZEND_ASSERT(0);
			return;
	}
}

zend_bool immutable_cache_unpersist(zval *dst, const zval *value, immutable_cache_serializer_t *serializer) {
	immutable_cache_unpersist_context_t ctxt;

	if (Z_TYPE_P(value) == IS_PTR) {
		return immutable_cache_unpersist_serialized(dst, Z_PTR_P(value), serializer);
	}

	ctxt.memoization_needed = 0;
	ZEND_ASSERT(Z_TYPE_P(value) != IS_REFERENCE);
	if (Z_TYPE_P(value) == IS_ARRAY) {
		ctxt.memoization_needed = 1;
		zend_hash_init(&ctxt.already_copied, 0, NULL, NULL, 0);
	}

	ZVAL_COPY_VALUE(dst, value);
	immutable_cache_unpersist_zval(&ctxt, dst);

	if (ctxt.memoization_needed) {
		zend_hash_destroy(&ctxt.already_copied);
	}
	return 1;
}

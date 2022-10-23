<?php

/**
 * @generate-function-entries
 * @generate-legacy-arginfo
 */

/**
 * @strict-properties
 * @not-serializable
 */
final class ImmutableCacheIterator implements Iterator {
    /** @param array|string|null $search */
    public function __construct(
        $search = null,
        int $format = IMMUTABLE_CACHE_ITER_ALL,
        int $chunk_size = 0,
        int $list = IMMUTABLE_CACHE_LIST_ACTIVE);

    public function rewind(): void;

    public function next(): void;

    public function valid(): bool;

    public function key(): string|int;

    public function current(): mixed;

    public function getTotalHits(): int;

    public function getTotalSize(): int;

    public function getTotalCount(): int;
}

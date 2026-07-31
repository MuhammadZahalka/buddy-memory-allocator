# Buddy Memory Allocator

A custom dynamic memory allocator in C++ implementing a buddy allocation scheme, developed incrementally across three versions from a naive `sbrk` wrapper to a full splitting/merging allocator with `mmap` support for large blocks.

## Overview

The allocator manages memory in power-of-two sized blocks organized into order-indexed free lists. Allocation finds the smallest order that fits and recursively splits larger blocks; freeing recursively merges a block with its buddy when both are free, reducing external fragmentation.

## Versions

| File | Description |
|---|---|
| `malloc_1.cpp` | Baseline — a minimal `sbrk`-based allocator with no reuse |
| `malloc_2.cpp` | Adds block metadata, a linked free list, and block reuse (`scalloc`, `sfree`, `srealloc`) |
| `malloc_3.cpp` | Full buddy allocator — order-based free lists, splitting, buddy merging, and `mmap` for large allocations |

## Design (malloc_3)

- **Initialization** — reserves an initial pool of 32 blocks of 128KB via `sbrk`, aligned and registered at the maximum order.
- **Free lists by order** — an array of linked lists indexed by block order (0–10), where order *n* holds blocks of size `128 * 2^n` bytes.
- **Splitting** — on allocation, the smallest sufficient block is recursively halved until further splitting would no longer fit the request.
- **Buddy merging** — on free, adjacent buddy blocks are recursively coalesced back into larger orders.
- **Large allocations** — requests exceeding 128KB bypass the buddy system and are served directly by `mmap` / `munmap`.
- **Reallocation** — `srealloc` attempts in-place merging with neighbouring free buddies before falling back to allocate-and-copy.

## API

```cpp
void* smalloc(size_t size);                  // allocate
void* scalloc(size_t num, size_t size);      // allocate and zero
void  sfree(void* p);                        // free
void* srealloc(void* oldp, size_t size);     // resize
```

Statistics helpers are also exposed for inspecting allocator state:

```cpp
size_t _num_free_blocks();
size_t _num_free_bytes();
size_t _num_allocated_blocks();
size_t _num_allocated_bytes();
size_t _num_meta_data_bytes();
size_t _size_meta_data();
```

## Notes

Written in C++ using `sbrk` and `mmap` directly, with no reliance on the standard library allocator. Developed as part of an Operating Systems course, focusing on memory management, fragmentation, and low-level systems programming.

// Automated tests for the buddy memory allocator (malloc_3).
// Built and run on every push by .github/workflows/ci.yml

#include <cstdio>
#include <cstring>
#include <cstdlib>

void*  smalloc(size_t size);
void*  scalloc(size_t num, size_t size);
void   sfree(void* p);
void*  srealloc(void* oldp, size_t size);
size_t _num_free_blocks();
size_t _num_free_bytes();
size_t _num_allocated_blocks();
size_t _num_allocated_bytes();
size_t _num_meta_data_bytes();
size_t _size_meta_data();

static int failures = 0;
static int checks   = 0;

static void check(bool cond, const char* name) {
    ++checks;
    if (cond) {
        printf("  pass  %s\n", name);
    } else {
        printf("  FAIL  %s\n", name);
        ++failures;
    }
}

static void test_rejects_invalid_sizes() {
    printf("rejects invalid sizes\n");
    check(smalloc(0) == NULL,            "smalloc(0) returns NULL");
    check(smalloc(100000001) == NULL,    "smalloc(> 1e8) returns NULL");
    check(scalloc(0, 10) == NULL,        "scalloc with num = 0 returns NULL");
    check(scalloc(10, 0) == NULL,        "scalloc with size = 0 returns NULL");
}

static void test_allocates_usable_memory() {
    printf("allocates usable memory\n");
    char* p = (char*)smalloc(100);
    check(p != NULL, "smalloc(100) returns a pointer");
    if (!p) return;

    memset(p, 'A', 100);
    bool intact = true;
    for (int i = 0; i < 100; ++i) {
        if (p[i] != 'A') { intact = false; break; }
    }
    check(intact, "written bytes read back unchanged");
    sfree(p);
}

static void test_scalloc_zeroes_memory() {
    printf("scalloc zeroes memory\n");
    unsigned char* p = (unsigned char*)scalloc(50, sizeof(int));
    check(p != NULL, "scalloc returns a pointer");
    if (!p) return;

    bool all_zero = true;
    for (size_t i = 0; i < 50 * sizeof(int); ++i) {
        if (p[i] != 0) { all_zero = false; break; }
    }
    check(all_zero, "scalloc memory is zero-filled");
    sfree(p);
}

static void test_free_updates_statistics() {
    printf("free updates statistics\n");
    size_t free_before = _num_free_blocks();
    void*  p = smalloc(64);
    check(p != NULL, "allocation for free test succeeded");
    if (!p) return;

    sfree(p);
    check(_num_free_blocks() >= free_before, "free block count does not decrease after sfree");
    sfree(p);
    check(true, "double sfree does not crash");
}

static void test_srealloc_preserves_contents() {
    printf("srealloc preserves contents\n");
    char* p = (char*)smalloc(32);
    check(p != NULL, "initial allocation succeeded");
    if (!p) return;

    strcpy(p, "buddy allocator test string");
    char* q = (char*)srealloc(p, 256);
    check(q != NULL, "srealloc returns a pointer");
    if (!q) return;

    check(strcmp(q, "buddy allocator test string") == 0, "contents survive srealloc");
    check(srealloc(NULL, 64) != NULL, "srealloc(NULL, n) behaves like smalloc");
    sfree(q);
}

static void test_large_allocation() {
    printf("large allocation path\n");
    size_t big = 256 * 1024;
    char* p = (char*)smalloc(big);
    check(p != NULL, "large allocation succeeds");
    if (!p) return;

    memset(p, 'Z', big);
    check(p[big - 1] == 'Z', "last byte of large block is writable");
    sfree(p);
}

static void test_metadata_consistency() {
    printf("metadata consistency\n");
    check(_size_meta_data() > 0, "metadata size is non-zero");
    check(_num_meta_data_bytes() == _num_allocated_blocks() * _size_meta_data(),
          "metadata bytes equal allocated blocks times metadata size");
    check(_num_free_blocks() <= _num_allocated_blocks(),
          "free blocks never exceed allocated blocks");
    check(_num_free_bytes() <= _num_allocated_bytes(),
          "free bytes never exceed allocated bytes");
}

int main() {
    printf("Buddy allocator test suite\n\n");

    test_rejects_invalid_sizes();
    test_allocates_usable_memory();
    test_scalloc_zeroes_memory();
    test_free_updates_statistics();
    test_srealloc_preserves_contents();
    test_large_allocation();
    test_metadata_consistency();

    printf("\n%d checks, %d failed\n", checks, failures);
    return failures == 0 ? 0 : 1;
}

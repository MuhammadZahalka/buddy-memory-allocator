#include <unistd.h>
#include <cstring>

const size_t MAX = 100000000;
size_t _num_free_blocks();
size_t _num_free_bytes();
size_t _num_allocated_blocks();
size_t _num_allocated_bytes();
size_t _num_meta_data_bytes();
size_t _size_meta_data();
//////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct MallocMetadata {
    size_t size;
    bool is_free;
    MallocMetadata* next;
    MallocMetadata* prev;
}Meta;
//////////////////////////////////////////////////////////////////////////////////////////////////////////
size_t num_free_blocks = 0;
size_t num_free_bytes = 0;
size_t num_allocated_blocks = 0;
size_t num_allocated_bytes = 0;
Meta* first = nullptr;
Meta* last = nullptr;
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void* smalloc(size_t size) {
        if (size == 0) {
            return nullptr;
        }
        if(size > MAX){
            return nullptr;
        }
        Meta* alloc = first;
        while (alloc != nullptr) {
            if (alloc->is_free && alloc->size >= size) {
                num_free_bytes -= alloc->size;
                alloc->is_free = false;
                num_free_blocks--;
                return (char*)alloc + sizeof(Meta);
            }
            alloc = alloc->next;
        }
        void* ret = sbrk(size + sizeof(Meta));
        if (ret == (void*)(-1)) {
            return nullptr;
        }

        Meta * newAlloc = (Meta *)ret;
        newAlloc->size = size;
        newAlloc->is_free = false;
        newAlloc->next = nullptr;
        newAlloc->prev = last;
        if (first == nullptr) {
            first = newAlloc;
            last = newAlloc;
        } else {
            last->next = newAlloc;
            last = newAlloc;
        }
        num_allocated_blocks++;
        num_allocated_bytes += size;

        return (char*)ret + sizeof(Meta);
}

void* scalloc(size_t num, size_t size) {
    if(size==0){
        return nullptr;
    }
    if(num==0){
        return nullptr;
    }
    size_t amount = size*num;
    if(amount>MAX){
        return nullptr;
    }
    void* space = smalloc(amount);
    if(space == nullptr)
        return nullptr;
    memset(space,0,amount);
    return space;
}
void sfree(void* p) {
    if(!p) {
        return;
    }
    Meta* alloc = (Meta *)((char*)p - sizeof(Meta));
    if(alloc -> is_free) {
        return;
    }
    num_free_bytes += alloc->size;
    alloc ->is_free = true;
    num_free_blocks++;
}
void* srealloc(void* oldp, size_t size) {
        if (oldp == nullptr) {
        return smalloc(size);
        }
        if (size == 0) {
            return nullptr;
        }
        if(size > MAX){
            return nullptr;
        }
        Meta * alloc = (Meta*)((char*)oldp - sizeof(Meta));
        if (alloc->size >= size) {
            return oldp;
        }
        void* newp = smalloc(size);
        if (!newp) {
            return nullptr;
        }
        else {
            memmove(newp, oldp, alloc->size);
            sfree(oldp);
        }
        return newp;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
size_t _num_free_blocks(){
    return num_free_blocks;
}
size_t _num_free_bytes(){
    return num_free_bytes;
}

size_t _num_allocated_blocks(){
    return num_allocated_blocks;
}
size_t _num_allocated_bytes(){
    return num_allocated_bytes;
}

size_t _num_meta_data_bytes(){
    return num_allocated_blocks * sizeof(Meta);
}

size_t _size_meta_data(){
    return sizeof(Meta);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
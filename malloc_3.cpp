#include <unistd.h>
#include <string.h>
#include <cmath>
#include <iostream>
#include <sys/mman.h>
#include <cstdint>
#include <stdint.h>
const size_t MAX = 100000000;
const int MAX_POWER = 10;
const size_t MAX_SIZE = 128 * 1024;
const size_t MIN_SIZE = 128;
const int MAX_BLOCKS = 32;
const int OFF = (int)pow(3, 10 + 1) - 1;
const int MAX_ALLOCATION = (int)pow(10, 8);
///----------------------------------malloc_2.cpp with additions-------------------------------------///
typedef struct MallocMetadata {
    size_t size;
    bool is_free;
    long order;
    MallocMetadata* next;
    MallocMetadata* prev;
}Meta;
///------------------------------------------end malloc_2.cpp-----------------------------------------///
class MemoryBlocks{

    
public:

	size_t num_free_blocks;
    size_t num_free_bytes;
    size_t num_allocated_blocks;
    size_t num_allocated_bytes;
    bool isisInitialized;
    
    Meta* array[MAX_POWER + 1];
    MemoryBlocks():num_free_blocks(0),num_free_bytes(0),num_allocated_blocks(0),num_allocated_bytes(0),isisInitialized(false){
        int i = 0;
        while(i<MAX_POWER + 1){
            array[i]= nullptr;
            i++;
        }
    }
    size_t getFreeBlocks();
    size_t getFreeBytes();
    size_t getAllocatedBlocks();
    size_t getAllocatedBytes();
    Meta* getBlock(int index);
    bool getInitialization();
    void setFreeBlocks(size_t size);
    void setFreeBytes(size_t size);
    void setAllocatedBlocks(size_t size);
    void setAllocatedBytes(size_t size);
    void setEQFreeBlocks(size_t size);
    void setEQFreeBytes(size_t size);
    void setEQAllocatedBlocks(size_t size);
    void setEQAllocatedBytes(size_t size);
    void setInitialization(bool flag);
    int splitMemory(Meta *memory,int order, size_t size);
    void* alloced(size_t size);
    void add(Meta* block,int order);
    int smallestOrderFits(size_t size);
    void mergeTwoBlocks(Meta *memory ,int order);
    void* merge(Meta *memory, size_t size);
    void* mergeAux(Meta *memory, size_t size);
    size_t maxOrder(int place, long index);
    void deleteMem(Meta* tmp,int order);
    };
size_t MemoryBlocks::getFreeBlocks(){
    return num_free_blocks;
}
size_t MemoryBlocks::getFreeBytes(){
    return num_free_bytes;
}
size_t MemoryBlocks::getAllocatedBlocks(){
    return num_allocated_blocks;
}
size_t MemoryBlocks::getAllocatedBytes(){
    return num_allocated_bytes;
}
Meta* MemoryBlocks::getBlock(int index){
    return (index >= 0 && index <= MAX_POWER) ? array[index] : nullptr;
}
bool MemoryBlocks::getInitialization() {
    return isisInitialized;
}
void MemoryBlocks::setFreeBlocks(size_t size){
    num_free_blocks += size;
}
void MemoryBlocks::setFreeBytes(size_t size) {
    num_free_bytes += size;
}
void MemoryBlocks::setAllocatedBlocks(size_t size){
    num_allocated_blocks += size;
}
void MemoryBlocks::setAllocatedBytes(size_t size){
    num_allocated_bytes += size;
}
void MemoryBlocks::setInitialization(bool flag){
isisInitialized = flag;
}
void MemoryBlocks::setEQFreeBlocks(size_t size){
    num_free_blocks = size;
}
void MemoryBlocks::setEQFreeBytes(size_t size){
    num_free_bytes = size;
}
void MemoryBlocks::setEQAllocatedBlocks(size_t size) {
    num_allocated_blocks = size;
}
void MemoryBlocks::setEQAllocatedBytes(size_t size) {
    num_allocated_bytes = size;
}
void MemoryBlocks::deleteMem(Meta* tmp,int order){
    if(tmp->next != nullptr)
        tmp->next->prev = tmp->prev;

    if(tmp->prev != nullptr)
        tmp->prev->next = tmp->next;
    else
        this->array[order] = tmp->next;

    tmp->next = nullptr;
    tmp->prev = nullptr;
}
int MemoryBlocks::splitMemory(Meta *memory,int order, size_t size){
        if (!memory || order < 0) {
            return -1;
        }
        if(order == 0){
			return 0;
		}
        size_t cut = (memory->size)/ 2 - sizeof(Meta)/2;

            if (size > cut) {
                return order;
            }

            Meta* firstHalf = memory;
            Meta* secondHalf = (Meta*)((char*)memory + (memory->size + sizeof(Meta)) / 2);

            if (!(firstHalf->prev)) {
                array[order] = firstHalf->next;
            } else {
                firstHalf->prev->next = firstHalf->next;
            }
            if (firstHalf->next != nullptr) {
                firstHalf->next->prev = firstHalf->prev;
            }


            firstHalf->size = cut;
            secondHalf->size = cut;
            secondHalf->order = firstHalf->order*3 + 1;
            firstHalf->order = firstHalf->order * 3;
            firstHalf->next = nullptr;
            firstHalf->prev = nullptr;
            secondHalf->is_free = true;

            add(firstHalf, order - 1);
            add(secondHalf, order - 1);

            num_free_blocks++;
            num_allocated_blocks++;
            num_allocated_bytes -= sizeof(Meta);
            num_free_bytes -= sizeof(Meta);

    return splitMemory(firstHalf,order - 1, size);
}
/*
int MemoryBlocks::smallestOrderFits(size_t size) {
    size_t block_size = 1;
    for (int pwr = 0; pwr <= MAX_POWER; pwr++) {
        if (block_size >= size) {
            return pwr;
        }
        block_size *= 2;
    }
    return -1;
}
* */

int MemoryBlocks::smallestOrderFits(size_t size) {
    size_t val = 1;
    for(int pwr = 0; pwr <= MAX_POWER; pwr++, val *= 2)
    {
        if(val * MIN_SIZE < size) {
            continue;
        }else
            return pwr;
    }
    return -1;
}


void* MemoryBlocks::alloced(size_t size) {
        int placement = smallestOrderFits(size + sizeof(Meta));
        Meta* memory = nullptr;
        int i = 0;
        for (i = placement; i <= MAX_POWER; i++) {
            if (array[i] != nullptr) {
                memory = array[i];
                break;
            }
        }
        if (!memory) {
            return nullptr;
        }
        int split = splitMemory(memory,i,size);

        num_free_blocks--;
        num_free_bytes -= memory->size;
        deleteMem(memory,split);
        memory->is_free = false;
        return memory;
}
void MemoryBlocks::add(Meta* block,int order){
        Meta* alloc = array[order];
        block->next = nullptr;
        block->prev = nullptr;
        if (!alloc) {
            array[order] = block;
            return;
        }
        bool beforeNext = false;
        while (alloc) {
            if (block->order < alloc->order) {
                break;
            }
            if (alloc->next == nullptr) {
                beforeNext = true;
                break;
            }
            alloc = alloc->next;
        }

        if (beforeNext) {
            alloc->next = block;
            block->prev = alloc;
        } else {
            if (alloc->prev != nullptr) {
                alloc->prev->next = block;
                block->prev = alloc->prev;
            } else {
                array[order] = block;
            }
            alloc->prev = block;
            block->next = alloc;
        }
        block->is_free = true;
}
void MemoryBlocks::mergeTwoBlocks(Meta *memory ,int order) {
    Meta* right = memory->next;
    deleteMem(memory,order);
    deleteMem(right,order);

    memory->size += right->size + sizeof(Meta);
    memory->order = memory->order / 3;


    add(memory, order + 1);


    num_free_blocks--;
    num_allocated_blocks--;
    num_free_bytes += sizeof(Meta);
    num_allocated_bytes+= sizeof(Meta);
}
void* MemoryBlocks::mergeAux(Meta *memory, size_t size){
        int desired = smallestOrderFits(size + sizeof(Meta));
        int currentOrder = smallestOrderFits(memory->size + sizeof(Meta));

        num_free_bytes += memory->size;
        num_free_blocks++;

        add(memory, currentOrder);

        long index = memory->order;
        int i = currentOrder;
        while(i<desired){
            Meta* left = array[i];
            while (left->order - index != 0 && left->order - index != -1) {
                left = left->next;
            }
            mergeTwoBlocks(left, i);
            index = index / 3;
            i++;
        }

        Meta * curr = array[desired];
        while (curr->order != index) {
            curr = curr->next;
        }

    deleteMem(curr,desired);
        num_free_blocks--;
        num_free_bytes -= curr->size;
        curr->is_free = false;

        return curr;
}
void* MemoryBlocks::merge(Meta *memory, size_t size) {
    int smallestOrder = smallestOrderFits(memory->size+ sizeof(Meta));
    if(maxOrder(smallestOrder,memory->order)<size)
        return nullptr;

    return mergeAux(memory,size);
}

size_t MemoryBlocks::maxOrder(int place, long index){
        if (place == MAX_POWER) {
            return MAX_SIZE;
        }
        else {
            Meta *curr = this->array[place];
            while (curr) {
                if (abs(curr->order - index) != 1) {
                    curr = curr->next;
                    continue;
                }
                return maxOrder(place + 1, index / 3);
            }
            return MIN_SIZE * pow(2, place);
        }
}
///--------------------------------------changes in malloc_2.cpp func-------------------------------------///
MemoryBlocks alloc = MemoryBlocks();
bool isEmpty() {
    alloc.setInitialization(true);
    void* newp = sbrk((int)(32 * MAX_SIZE));
    if (newp == (void*)(-1) || newp == nullptr) {
        return false;
    }
    int i = 0;
    void* tmp = newp;
    while (i < 32) {
        Meta* memory = (Meta*)tmp;
        tmp = (void*)((char*)tmp + MAX_SIZE);
        memory->order = OFF * i + 1;
        memory->size = MAX_SIZE - sizeof(Meta);
        alloc.add(memory, MAX_POWER);
        i++;
    }
    alloc.setEQFreeBlocks(32);
    alloc.setEQAllocatedBlocks(32);
    alloc.setEQFreeBytes(32 * MAX_SIZE - 32 * sizeof(Meta));
    alloc.setEQAllocatedBytes(32 * MAX_SIZE - 32 * sizeof(Meta));
    return true;
}

void* smalloc(size_t size) {
     if (alloc.getInitialization() == false) {
        if (!isEmpty()) {
            return nullptr;
        }
    }
    if (size == 0) {
        return nullptr;
    }
    if (size > MAX) {
        return nullptr;
    }
   
    void* newp = nullptr;
    if (size > MAX_SIZE - sizeof(Meta)) {
        newp = mmap(nullptr, size + sizeof(Meta), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (newp == MAP_FAILED) {
            return nullptr;
        }
        alloc.setAllocatedBlocks(1);
        alloc.setAllocatedBytes(size);
        Meta* memoryManaged = (Meta*)newp;
        memoryManaged->size = size;
        memoryManaged->is_free = false;
        memoryManaged->prev = nullptr;
        memoryManaged->next = nullptr;
    }
    else {
        newp = alloc.alloced(size);
    }
    if (!newp) {
        return nullptr;
    }
    return (char*)newp + sizeof(Meta);
}
void* scalloc(size_t number ,size_t size){
    if(size * number > MAX){
        return nullptr;
    }
    if(size==0){
        return nullptr;
    }
    if(number==0){
        return nullptr;
    }
    void* newp = smalloc(number*size);
    if(!newp)
    {
        return nullptr;
    }
    memset(newp,0,number * size);
    return newp;
}

void mergeAdjacentFreeBlocks(){
    int i=0;
    while(i<MAX_POWER){
        Meta* memory = alloc.array[i];
        while((memory)&& (memory->next))
        {
            if(memory->order + 1 == memory->next->order) {
                alloc.mergeTwoBlocks(memory, i);
            }
            memory = memory->next;
        }
        i++;
    }
}
void sfree(void* p) {
    if (!p) {
        return;
    }
    Meta* memory = (Meta*)((char*)p - sizeof(Meta));
    if (memory->is_free) {
        return;
    }
    if (memory->size <= MAX_SIZE - sizeof(Meta)) {
        alloc.setFreeBlocks(1);
        alloc.setFreeBytes(memory->size);
        memory->is_free = true;
        int order = alloc.smallestOrderFits(memory->size + sizeof(Meta));
        alloc.add(memory, order);
        mergeAdjacentFreeBlocks();
    } else {
        alloc.setAllocatedBytes(-memory->size);
        alloc.setAllocatedBlocks(-1);
        size_t allocated = memory->size + sizeof(Meta);
        munmap((void*)memory, allocated);
    }
}
void* srealloc(void* oldp, size_t size) {
	if (size == 0) {
        return nullptr;
    }
    if(size > MAX_ALLOCATION){
        return nullptr;
    }
    if (oldp == nullptr) {
        return smalloc(size);
    }
    Meta* memory = (Meta*)((char*)oldp - sizeof(Meta));
    void* newp = nullptr;
    bool isMerged = false;
    if (memory->size <= MAX_SIZE - sizeof(Meta)) {
        if (memory->size >= size) {
            return oldp;
        }
        newp = alloc.merge(memory, size);
        if (newp != nullptr) {
            newp = (char*)newp + sizeof(Meta);
        } else {
            isMerged = true;
            newp = smalloc(size);
            if (!newp) {
                return nullptr;
            }
        }
        memmove(newp, oldp, memory->size);
        if (isMerged) {
            sfree(oldp);
        }
    } else {
        if (memory->size == size) {
            return oldp;
        }
        newp = smalloc(size);
        if (!newp) {
            return nullptr;
        }
        size_t copyBytes = memory->size;
        if (size < memory->size) {
            copyBytes = size;
        }
            memmove(newp, oldp, copyBytes);
            sfree(oldp);
    }
    return newp;
}
size_t _num_free_blocks(){
    return alloc.getFreeBlocks();
}
size_t _num_free_bytes(){
    return alloc.getFreeBytes();
}
size_t _num_allocated_blocks(){
    return alloc.getAllocatedBlocks();
}
size_t _num_allocated_bytes(){
    return alloc.getAllocatedBytes();
}
size_t _num_meta_data_bytes(){
    return alloc.getAllocatedBlocks() * sizeof(Meta);
}
size_t _size_meta_data(){
    return sizeof(Meta);
}

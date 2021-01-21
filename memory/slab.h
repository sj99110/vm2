#pragma once

#define BLOCKSIZE 4096

#include <stdint.h>
#include <cstddef>

#define GET_SIZE(flags) flags & 127
#define GET_FREE(flags) flags & 128
#define SET_FREE(flags) flags | 128
#define SET_SIZE(flags, i) flags | (127 | i)
#define CLEAR_FREE(flags) flags ^ 128
struct Object
{
    uint8_t flags;
    char *data;
    char& operator*()
    {
        return *data;
    }
    
    char& operator[](uint32_t i)
    {
       return data[i];
    }
    
    char *getPrim()
    {
        return data;
    }
    
}__attribute__((packed));

struct Slab
{
    Slab *p, *n;
    Object *objDefs;
    uint8_t objSize, objCount;
    char free; // 0 full, 1 partial, 2 empty
} __attribute__((packed));

struct SlabCache
{
    Slab *partial, *full, *empty;
    unsigned int size, objSize;
    size_t pageSize, slabSize;
};

class SlabAllocator
{
private:
    SlabCache cache;
    uint32_t pageSize, objSize;
    Slab* createSlab();
    void initCache();
    Object *getFirst();
    Object *getContinuous(size_t reqBlocks);
public:
    SlabAllocator();
    SlabAllocator(size_t pageSize, size_t oSize);
    ~SlabAllocator();
    Object *slabAlloc(size_t size);
    int slabFree(Object*);
};
#include <sys/mman.h>
#include <stdint.h>
#include <cstddef>
#include <iostream>
#include <errno.h>

#include "slab.h"

SlabAllocator::SlabAllocator()
{
    SlabAllocator(4096, 8);
}

SlabAllocator::SlabAllocator(size_t size, size_t oSize)
{
    pageSize = size;
    objSize = oSize;
    initCache();
    Slab *newSlab = createSlab();
    cache.empty = newSlab;
    cache.size += 1;
}

void SlabAllocator::initCache()
{
    cache.full = NULL;
    cache.partial = NULL;
    cache.size = 0;
    cache.pageSize = pageSize;
    cache.slabSize = pageSize;
    cache.objSize = objSize;
    cache.empty = createSlab();
}

Slab *SlabAllocator::createSlab()
{
    Object *newObject;
    Slab *newSlab = (Slab*)mmap(0, pageSize*sizeof(char), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
    if(newSlab == MAP_FAILED)
        std::cout<<"alloc failure "<<errno<<"\n";
    newSlab->p = NULL;
    newSlab->n = NULL;
    newSlab->free = 2;
    newSlab->objSize = objSize;
    char *addr = (char*)newSlab + sizeof(Slab);
    newSlab->objDefs = (Object*)addr;
    int objNum = (pageSize-sizeof(Slab)) / (objSize+sizeof(Object));
    uintptr_t dataOffset = sizeof(Slab) + objNum * sizeof(Object);
    newSlab->objCount = objNum;
    newObject = (Object*)addr;
    for(int i=0;i<objNum-1;i++)
    {
        newObject += 1;
        newObject->data = (char*)newSlab + dataOffset + (i * objSize);
        newObject->flags = 0;
        newObject->flags = SET_FREE(newObject->flags);
        newObject->flags = SET_SIZE(newObject->flags, 1);
    }
    return newSlab;
}

Object *SlabAllocator::slabAlloc(size_t size)
{
    int objSize = cache.objSize;
    int rem = size % objSize;
    if(size <= objSize)
    {
        return getFirst();
    }
    size_t reqBlocks = (size / objSize);
    if(rem)
        reqBlocks += rem;
    if(reqBlocks == 1)
        return getFirst();
    return getContinuous(reqBlocks);
}

Object *SlabAllocator::getFirst()
{
    Slab *slab;
    Object *newObject;
    if(!cache.empty && !cache.partial)
    {
        cache.empty = createSlab();
        cache.size++;
    }
    if(cache.partial)
        slab = cache.partial;
    else
        slab = cache.empty;
    newObject = slab->objDefs;
    for(int i=0;i<slab->objCount;i++)
    {
        if(GET_FREE(newObject->flags))
        {
            newObject->flags = CLEAR_FREE(newObject->flags);
            return newObject;
        }
        newObject++;
    }
    return 0;
}

Object *SlabAllocator::getContinuous(size_t reqBlocks)
{
    Slab *slab;
    Object *newObject, *tmpObject;
    int count=0;
    if(!cache.empty && !cache.partial)
    {
        cache.empty = createSlab();
        cache.size++;
    }
    if(cache.partial)
        slab  = cache.partial;
    else
        slab = cache.empty;
    newObject = slab->objDefs;
    for(int i=0,count=0;i<slab->objCount;i++)
    {
        if(GET_FREE(newObject->flags))
        {
            count++;
            for(int j=1;j<reqBlocks;j++)
            {
                tmpObject = newObject + j;
                if(GET_FREE(tmpObject->flags))
                    count++;
            }
            if(count == reqBlocks)
            {
                newObject->flags = SET_SIZE(newObject->flags, reqBlocks);
                newObject->flags = CLEAR_FREE(newObject->flags);
                for(int j=1;j++;j<reqBlocks)
                {
                    tmpObject = newObject + j;
                    tmpObject->flags = CLEAR_FREE(tmpObject->flags);
                    tmpObject->flags = SET_SIZE(tmpObject->flags, 0);
                    if(j >= reqBlocks)
                        break;
                }
                return newObject;
            }
        }
        newObject++;
    }
    return 0;
}

int SlabAllocator::slabFree(Object *object)
{
    Object *newObject = object;
    int size = GET_SIZE(object->flags);
    for(int i=0;i++;i<size)
    {
        newObject = newObject + (i * sizeof(Object));
        newObject->flags = SET_FREE(newObject->flags);
        newObject->flags = SET_SIZE(newObject->flags, 1);
    }
    return 1;
}

SlabAllocator::~SlabAllocator()
{}

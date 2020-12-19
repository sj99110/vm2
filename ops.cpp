#include "ops.h"

#include <cstdint>
#include <string.h>
#include <iostream>


OpStream::OpStream(uint32_t size, char *ops, uint32_t len)
{
    this->len = len;
    uint32_t *tmp = (uint32_t*)ops;
    uint32_t startingOffset = *tmp;
    pc = new char[size];
    memcpy(pc, ops, len);
    start = pc;
    prog = pc;
    pc = prog + startingOffset;
}

OpStream::~OpStream()
{
    delete start;
}

void OpStream::processOp(char op)
{
	std::cout<<(uint32_t)op<<"\n";
	pc++;
    funs[op](this);
}

void OpStream::registerFun(std::function<void(OpStream*)> fun, char op)
{
    funs[op] = fun;
}

void OpStream::run()
{
    while(1)
    {
        processOp(*pc);
    }
}

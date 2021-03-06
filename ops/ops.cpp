#include "ops.h"
#include "SymTable.h"

#include <cstdint>
#include <string.h>
#include <iostream>
#include <utility>
#include <map>
#include <exception>

OpStream::OpStream(uint32_t size, char *ops, uint32_t len)
{
    
    this->len = len;
    pc = new char[size];
    memcpy(pc, ops, len);
    uint32_t offset;
    auto pair = buildSymTable(pc);
    std::tie<uint32_t, std::map<std::string, uint32_t>>(offset, childFuns) = pair;
    memset(regs, 0, sizeof(uintptr_t) * 15);
    start = pc;
    prog = pc + offset;
    pc = prog;
}

OpStream::~OpStream()
{
    delete start;
    childFuns.clear();
    hostFuns.clear();
}

void OpStream::processOp(char op)
{
	pc++;
    funs[op](this);
}

void OpStream::registerFun(std::function<void(OpStream*)> fun, char op)
{
    funs[op] = fun;
}

void OpStream::bindHostFun(std::string name, std::function<void(OpStream*)> fun)
{
    hostFuns[name] = fun;
}

void OpStream::unbindHostFun(std::string name)
{
    hostFuns.erase(name);
}

void OpStream::callVMFun(std::string name)
{
    if(!childFuns.contains(name))
    {
        std::cerr<<"no function " + name + " in child lookup table\n";
        return;
    }
    uint32_t offset = childFuns[name];
    regs[15] = (uintptr_t)pc;
    pc = prog + offset;
    run(RET);
    pc = (char*)regs[15];
    regs[15] = 0;
}

void OpStream::run(OPS end)
{
    if(!childFuns.contains("main"))
        std::invalid_argument("no main function found");
    pc = prog + childFuns["main"];
    while(1)
    {
        if(end == *pc)
            break;
        processOp(*pc);
    }
}

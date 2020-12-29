#include <iostream>
#include <functional>
#include <cstring>
#include <string>
#include <list>
#include <utility>
#include <sys/mman.h>
#include <exception>
#include <fstream>

#include "slab.h"
#include "ops.h"
#include "funs.h"
#include "ffi.h"
#include "SymTable.h"

void test(OpStream *os)
{
    os->pc++;
    std::cout<<(int)*os->prog<<std::endl;
    os->pc = os->prog;
}

void putStr(Object obj, const char* st)
{
    int count=0;
    while(st[count] != '\0')
    {
        obj[count] = st[count];
        count++;
    }
}

char *putName(char *pc, const char *name)
{
	strcpy(pc, name);
	pc += strlen(name)+1;
	return pc;
}

char *setReg(char *pc, char reg, uint32_t data)
{
	uint32_t *tmp;
	*pc = MOV;
	pc++;
	*pc = reg;
	pc++;
	tmp = (uint32_t*)pc;
	*tmp = data;
	tmp++;
	return (char*)tmp;
}

char *setRegPtr(char *pc, char reg, uintptr_t data)
{
	uintptr_t *tmp;
	*pc = MOVQ;
	pc++;
	*pc = reg;
	pc++;
	tmp = (uintptr_t*)pc;
	*tmp = data;
	tmp++;
	return (char*)tmp;
}

char *ORR(char *pc, OPS op, char r1, char r2)
{
	*pc = (char)op;
	pc++;
	*pc = r1;
	pc++;
	*pc = r2;
	pc++;
	return pc;
}

char *OUINT(char *pc, OPS op, uint32_t dat)
{
	uint32_t *tmp;
	*pc = (char)op;
	pc++;
	tmp = (uint32_t*)pc;
	*tmp = dat;
	tmp++;
	return (char*)tmp;
}

char *OSTR(char *pc, OPS op, const char *str)
{
	*pc = (char)op;
	pc++;
	pc = putName(pc, str);
	return pc;
}

char *movReg(char *op, char reg1, char reg2)
{
	*op = MOVR;
	op++;
	*op = reg1;
	op++;
	*op = reg2;
	op++;
	return op;
}

uint32_t createSymTable(char *pc, std::list<std::pair<uint32_t, uint32_t>> syms)
{
    uint32_t entries = syms.size();
    uint32_t off, nameOff;
    SymTable *table = (SymTable*)pc;
    SymHeader header;
    SymTableEntry *_entries = table->entries;
    uint32_t offset = ((char*)_entries - pc) + (sizeof(SymTableEntry) * entries);
    header.entries = entries;
    header.MAGIC_NUMBER = 0x800C244;
    header.size = sizeof(SymTable) + (sizeof(SymTableEntry) * entries);
    header.version = 0;
    table->header = header;
    for(int i=0;i<entries;i++)
    {
        std::tie<uint32_t, uint32_t>(off, nameOff) = syms.back();
        _entries[i].type = 0;
        _entries[i].offset = off;
        _entries[i].name = nameOff;
        syms.pop_back();
    }
    return offset;
}

typedef std::pair<uint32_t, uint32_t> SymPair;

char *buildBytecode(char *proc, std::list<SymPair> symList, uint32_t size)
{
    uint32_t entries = symList.size();
    uint32_t symSize = sizeof(SymTable) + (sizeof(SymTableEntry) * entries);
    char *bytecode = new char[symSize + size];
    if(!bytecode)
        exit(-2);
    uint32_t pcOffset = createSymTable(bytecode, symList);
    memcpy(bytecode + pcOffset, proc, size);
    return bytecode;
}

char *buildOps1(char *ops)
{
    char *pc = ops;
    char *_main = pc;
    pc = putName(pc, "main");
    char *_addLoop = pc;
    pc = putName(pc, "addLoop");
    char *_add = pc;
    pc = putName(pc, "add");
    char *main = pc;
    pc = setReg(pc, 4, 10);
    setReg(pc, 0, 0);
    pc = setReg(pc, 1, 1);
    pc = OSTR(pc, CALL, "addLoop");
    *pc = HLT;
    pc++;
    char *addLoop = pc;
    char *jmp = pc;
    pc = OSTR(pc, CALL, "add");
    pc = ORR(pc, CMP, 0, 4);
    pc = OUINT(pc, JNE, jmp - ops);
    *pc = RET;
    pc++;
    char *addfun = pc;
    pc = ORR(pc, ADD, 0, 1);
    *pc = RET;
    *pc++;
    std::list<std::pair<uint32_t, uint32_t>> funList;
    funList.push_back(SymPair(main - ops, _main - ops));
    funList.push_back(SymPair(addLoop - ops, _addLoop - ops));
    funList.push_back(SymPair(addfun - ops, _add - ops));
    char *prog = buildBytecode(ops, funList, pc - ops);
    free(ops);
    return prog;
}

void hltFun(OpStream *os)
{
	int r1 = os->regs[0];
	exit(r1);
}

int main(int argc, char *argv[])
{
	char *ops = new char[512];
	char *prog = buildOps1(ops);
    std::ofstream file("file", std::ofstream::out);
    file.write(prog, 512);
    /*
    SlabAllocator slab(4096, 4);
    Object obj = *slab.slabAlloc(32);
    putStr(obj, "hello, world\0");
    std::cout<<obj.getPrim()<<"\n";
    slab.slabFree(&obj);
    */
    OpStream os = OpStream(1024, prog, 512);
    std::cout<<"binding funs\n";
    bindFunctions(&os);
    os.funs[HLT] = hltFun;
    std::cout<<"running stream\n";
    try {
		os.run(HLT);
	} catch(const std::exception& e) {
		std::cout<<e.what()<<" error\n";
        for(auto fun : os.childFuns)
        {
            std::cout<<"fun:\n";
            std::cout<<fun.first << " " << fun.second << "\n";
        }
        delete prog;
        return -1;
	}
    delete prog;
    return 1;
}

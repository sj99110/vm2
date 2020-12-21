#include <iostream>
#include <functional>
#include <cstring>
#include <string>

#include "slab.h"
#include "ops.h"
#include "funs.h"
#include "ffi.h"

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
	*pc = MOV;
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

void buildOps(uint32_t size, char* ops)
{
	char *ctmp;
	uint32_t *tmp;
	memset(ops, '0', size);
	char *pc = ops + 4;
	char *libNameOffset = pc;
	ctmp = ops;
	pc = putName(pc, "/usr/lib/x86_64-linux-gnu/libncurses.so.6.2");
	char *ncurses = pc;
	pc = putName(pc, "ncurses");
	char *initScr = pc;
	pc = putName(pc, "initscr");
	char *raw = pc;
	pc = putName(pc, "raw");
	char *keypad = pc;
	pc = putName(pc, "keypad");
	char *noecho = pc;
	pc = putName(pc, "noecho");
	char *printw = pc;
	pc = putName(pc, "printw");
	char *getch = pc;
	pc = putName(pc, "getch");
	char *arg1 = pc;
	pc = putName(pc, "%i");
	char *_start = pc;
	pc = setReg(pc, 0, libNameOffset - ctmp);
	pc = setReg(pc, 1, ncurses - ctmp);
	pc = ORR(pc, FFIBINDLIB, 0, 1);
	pc = setReg(pc, 0, initScr - ctmp);
	pc = setReg(pc, 1, ncurses - ctmp);
	pc = ORR(pc, FFIBINDFUN, 0, 1);
	pc = setReg(pc, 0, raw - ctmp);
	pc = setReg(pc, 1, ncurses - ctmp);
	pc = ORR(pc, FFIBINDFUN, 0, 1);
	pc = setReg(pc, 0, keypad - ctmp);
	pc = setReg(pc, 1, ncurses - ctmp);
	pc = ORR(pc, FFIBINDFUN, 0, 1);
	pc = setReg(pc, 0, noecho  - ctmp);
	pc = setReg(pc, 1, ncurses - ctmp);
	pc = ORR(pc, FFIBINDFUN, 0, 1);
	pc = setReg(pc, 0, printw - ctmp);
	pc = setReg(pc, 1, ncurses - ctmp);
	pc = ORR(pc, FFIBINDFUN, 0, 1);
	pc = setReg(pc, 0, getch - ctmp);
	pc = setReg(pc, 1, ncurses - ctmp);
	pc = ORR(pc, FFIBINDFUN, 0, 1);
	pc = setReg(pc, 4, 10);
	pc = setReg(pc, 0, 0);
	pc = setReg(pc, 1, 1);
	pc = setReg(pc, 4, 10);
	char *jmp = pc;
	pc = ORR(pc, ADD, 0, 1);
	pc = ORR(pc, CMP, 0, 4);
	pc = OUINT(pc, JNE, jmp - ctmp);
	pc = setReg(pc, 7, 0);
	pc = OSTR(pc, FFICALL, "initscr");
	pc = OSTR(pc, FFICALL, "raw");
	pc = OSTR(pc, FFICALL, "noecho");
	pc = movReg(pc, 0, 6);
	pc = setRegPtr(pc, 0, (uintptr_t)arg1);
	pc = movReg(pc, 6, 1);
	pc = setReg(pc, 7, 2);
	pc = OSTR(pc, FFICALL, "printw");
	pc = setReg(pc, 7, 0);
	pc = OSTR(pc, FFICALL, "getch");
	*pc = HLT;
	pc++;
	uint32_t *start = (uint32_t*)ops;
	*start = _start - ops;
	ops++;
}

void buildOps2(int size, char *ops)
{
	char *pc = ops+4;
	char *start = pc;
	pc = setReg(pc, 0, 0);
	pc = setReg(pc, 1, 1);
	pc = setReg(pc, 4, 10);
	char *jmp = pc;
	pc = ORR(pc, ADD, 0, 1);
	pc = ORR(pc, CMP, 0, 4);
	pc = OUINT(pc, JNE, jmp - ops);
	pc = OUINT(pc, HLT, 1);
	uint32_t *begin = (uint32_t*)ops;
	*begin = start - ops;
}

void hltFun(OpStream *os)
{
	int r1 = os->regs[0];
	exit(r1);
}

int main(int argc, char *argv[])
{
	char ops[512];
	buildOps(512, ops);
    SlabAllocator slab(4096, 4);
    Object obj = *slab.slabAlloc(32);
    putStr(obj, "hello, world\0");
    std::cout<<obj.getPrim()<<"\n";
    slab.slabFree(&obj);
    OpStream os = OpStream(1024, ops, 512);
    std::cout<<(uint32_t)MOV<<"\n";
    std::cout<<"binding funs\n";
    bindFunctions(&os);
    os.funs[HLT] = hltFun;
    std::cout<<"begin: "<<(uint32_t)*ops<<"\n";
    std::cout<<"running stream\n";
    try {
		os.run();
	} catch(const std::bad_function_call& e) {
		std::cout<<e.what()<<" error\n";
	}
    return 1;
}

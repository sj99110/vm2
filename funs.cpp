#include <stack>
#include <iostream>
#include <cstring>

#include "funs.h"
#include "ops.h"

char *getRegs(char *ops, int *reg1, int *reg2)
{
	*reg1 = (int)*ops;
	ops++;
	if(reg2 == NULL)
		return ops;
	*reg2 = (int)*ops;
	ops++;
	return ops;
}

void addFun(OpStream *os)
{
	int r1, r2;
	os->pc = getRegs(os->pc, &r1, &r2);
	os->regs[r1] += os->regs[r2];
}

void subFun(OpStream *os)
{
	int r1, r2;
	os->pc = getRegs(os->pc, &r1, &r2);
	os->regs[r1] -= os->regs[r2];
}

void mulFun(OpStream *os)
{
	int r1, r2;
	os->pc = getRegs(os->pc, &r1, &r2);
	os->regs[r1] *= os->regs[r2];
}

void divFun(OpStream *os)
{
	int r1, r2;
	os->pc = getRegs(os->pc, &r1, &r2);
	os->regs[r1] /= os->regs[r2];
}

void xorFun(OpStream *os)
{
	int r1, r2;
	os->pc = getRegs(os->pc, &r1, &r2);
	os->regs[r1] ^= os->regs[r2];
}

void andFun(OpStream *os)
{
	int r1, r2;
	os->pc = getRegs(os->pc, &r1, &r2);
	os->regs[r1] &= os->regs[r2];
}

void orFun(OpStream *os)
{
	int r1, r2;
	os->pc = getRegs(os->pc, &r1, &r2);
	os->regs[r1] |= os->regs[r2];
}

void lShiftFun(OpStream *os)
{
	int r1, r2;
	os->pc = getRegs(os->pc, &r1, &r2);
	uint32_t reg0 = os->regs[r1];
	uint32_t reg1 = os->regs[r2];
	reg0 = reg0 << reg1;
	os->regs[0] = reg0;
}

void rShiftFun(OpStream *os)
{
	int r1, r2;
	os->pc = getRegs(os->pc, &r1, &r2);
	uint32_t reg0 = os->regs[r1];
	uint32_t reg1 = os->regs[r2];
	reg0 = reg0 >> reg1;
	os->regs[0] = reg0;
}

void pushFun(OpStream *os)
{
	int r1;
	os->pc = getRegs(os->pc, &r1, NULL);
	uint32_t dat = os->regs[r1];
	os->progStack.push(dat);
}

void popFun(OpStream *os)
{
	int r1;
	os->pc = getRegs(os->pc, &r1, NULL);
	uint32_t dat = os->progStack.top();
	os->progStack.pop();
	os->regs[r1] = dat;
}

void ffiBindLib(OpStream *os)
{
	int r1, r2;
	char name[64], fName[64];
	char *nameLoc;
	os->pc = getRegs(os->pc, &r1, &r2);
	uint32_t offset = os->regs[r1];
	uint32_t nameOffset = os->regs[r2];
	nameLoc = os->prog + offset;
	strcpy(name, nameLoc);
	nameLoc = os->prog + nameOffset;
	strcpy(fName, nameLoc);
	os->ffi.openLib(name, fName);
}

void ffiBindFun(OpStream *os)
{
	int r1, r2;
	char name[64], fName[64], *nameLoc;
	os->pc = getRegs(os->pc, &r1, &r2);
	uint32_t offset = os->regs[r1];
	uint32_t nameOffset = os->regs[r2];
	nameLoc = os->prog + offset;
	strcpy(name, nameLoc);
	nameLoc = os->prog + nameOffset;
	strcpy(fName, nameLoc);
	os->ffi.loadFun(name, fName);
}

void ffiCallFun(OpStream *os)
{
	uint64_t r1, r2, r3, r4, r5, r6;
	uint64_t rVal = 0;
	char *fun = os->pc;
	char name[64];
	strcpy(name, fun);
	os->pc += 4;
	r1 = os->regs[0];
	r2 = os->regs[1];
	r3 = os->regs[2];
	r4 = os->regs[3];
	r5 = os->regs[4];
	r6 = os->regs[5];
	asm __volatile__ ("movq %1, %%rdi;"
		"movq %2, %%rsi;"
		"movq %3, %%rdx;"
		"movq %4, %%rcx;"
		"movq %5, %%r8;"
		"movq %6, %%r9;"
		"call %7;"
		"movq %%rax, %0;"
		: "=r"(rVal)
		: "r"(r1), "r"(r2), "r"(r3), "r"(r4), "r"(r5), "r"(r6), "r"(os->ffi.getFun(name))
		: "%rsi", "%rdx", "%rcx", "%r8", "%r9");
}

void jeFun(OpStream *os)
{
	uint32_t dat1 = os->regs[5];
	uint32_t *tmp = (uint32_t*)os->pc;
	os->pc += sizeof(uint32_t);
	if(dat1)
		os->pc = os->prog + *tmp;
}

void jneFun(OpStream *os)
{
	uint32_t dat1 = os->regs[5];
	uint32_t *tmp = (uint32_t*)os->pc;
	os->pc += sizeof(uint32_t);
	if(dat1 != 1)
		os->pc = os->prog + *tmp;
}

void movFun(OpStream *os)
{
	char *dat1;
	uint32_t *dat2;
	dat1 = os->pc;
	os->pc++;
	dat2 = (uint32_t*)os->pc;
	os->pc += sizeof(uint32_t);
	os->regs[*dat1] = *dat2;
	int i;
}

void movrFun(OpStream *os)
{
	int r1, r2;
	os->pc = getRegs(os->pc, &r1, &r2);
	os->regs[r1] = os->regs[r2];
}

void cmpFun(OpStream *os)
{
	int r1, r2;
	uint32_t dat1, dat2;
	os->pc = getRegs(os->pc, &r1, &r2);
	dat1 = os->regs[r1];
	dat2 = os->regs[r2];
	if(dat1 == dat2)
		os->regs[5] = 1;
	else if(dat1 > dat2)
		os->regs[5] = 2;
	else if(dat1 < dat2)
		os->regs[5] = 3;
	else
		os->regs[5] = 0;
}

void printOps(OpStream *os)
{
	char *ops = os->start;
	std::cout<<"printing ops\n";
	for(int i=0;i<os->len;i++)
		std::cout<<ops[i]<<"\n";
}

void testOutFun(OpStream *os)
{
	std::cout<<"printing registers\n";
	for(int i=0;i<15;i++)
		std::cout<<os->regs[i]<<"\n";
	while(1)
	{}
}

void errorFun(OpStream *os)
{
	std::cout<<"function not found\n";
}

void bindFunctions(OpStream *os)
{
	for(int i=0;i<64;i++)
		(*os)[i] = errorFun;
	(*os)[ADD] = addFun;
	(*os)[SUB] = subFun;
	(*os)[MUL] = mulFun;
	(*os)[DIV] = divFun;
	(*os)[XOR] = xorFun;
	(*os)[AND] = andFun;
	(*os)[OR] = orFun;
	(*os)[LSHIFT] = lShiftFun;
	(*os)[RSHIFT] = rShiftFun;
	(*os)[PUSH] = pushFun;
	(*os)[POP] = popFun;
	(*os)[FFIBINDLIB] = ffiBindLib;
	(*os)[FFIBINDFUN] = ffiBindFun;
	(*os)[FFICALL] = ffiCallFun;
	(*os)[JE] = jeFun;
	(*os)[JNE] = jneFun;
	(*os)[MOV] = movFun;
	(*os)[MOVR] = movrFun;
	(*os)[CMP] = cmpFun;
	(*os)[0] = testOutFun;
}

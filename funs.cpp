#include <stack>
#include <iostream>
#include <cstring>

#include "funs.h"
#include "ops.h"

char *getRegs(char *ops, char *reg1, char *reg2)
{
	*reg1 = *ops;
	ops++;
	if(reg2 == NULL)
		return ops;
	*reg2 = *ops;
	ops++;
	return ops;
}

void addFun(OpStream *os)
{
	char r1, r2;
	os->pc = getRegs(os->pc, &r1, &r2);
	os->regs[r1] += os->regs[r2];
}

void subFun(OpStream *os)
{
	char r1, r2;
	os->pc = getRegs(os->pc, &r1, &r2);
	os->regs[r1] -= os->regs[r2];
}

void mulFun(OpStream *os)
{
	char r1, r2;
	os->pc = getRegs(os->pc, &r1, &r2);
	os->regs[r1] *= os->regs[r2];
}

void divFun(OpStream *os)
{
	char r1, r2;
	os->pc = getRegs(os->pc, &r1, &r2);
	os->regs[r1] /= os->regs[r2];
}

void xorFun(OpStream *os)
{
	char r1, r2;
	os->pc = getRegs(os->pc, &r1, &r2);
	os->regs[r1] ^= os->regs[r2];
}

void andFun(OpStream *os)
{
	char r1, r2;
	os->pc = getRegs(os->pc, &r1, &r2);
	os->regs[r1] &= os->regs[r2];
}

void orFun(OpStream *os)
{
	char r1, r2;
	os->pc = getRegs(os->pc, &r1, &r2);
	os->regs[r1] |= os->regs[r2];
}

void lShiftFun(OpStream *os)
{
	char r1, r2;
	os->pc = getRegs(os->pc, &r1, &r2);
	uint32_t reg0 = os->regs[r1];
	uint32_t reg1 = os->regs[r2];
	reg0 = reg0 << reg1;
	os->regs[0] = reg0;
}

void rShiftFun(OpStream *os)
{
	char r1, r2;
	os->pc = getRegs(os->pc, &r1, &r2);
	uint32_t reg0 = os->regs[r1];
	uint32_t reg1 = os->regs[r2];
	reg0 = reg0 >> reg1;
	os->regs[0] = reg0;
}

void pushFun(OpStream *os)
{
	char r1;
	os->pc = getRegs(os->pc, &r1, NULL);
	uint32_t dat = os->regs[r1];
	os->progStack.push(dat);
}

void popFun(OpStream *os)
{
	char r1;
	os->pc = getRegs(os->pc, &r1, NULL);
	uint32_t dat = os->progStack.top();
	os->progStack.pop();
	os->regs[r1] = dat;
}

void ffiBindLib(OpStream *os)
{
	char r1, r2;
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
	char r1, r2;
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

void ffiCallFun1(void*, OpStream*);
void ffiCallFun2(void*, OpStream*);
void ffiCallFun3(void*, OpStream*);

void ffiCallFun(OpStream *os)
{
	std::cout<<"start fficall\n";
	uint64_t rVal = 0;
	char name[64];
	uint64_t r1 = os->regs[7];
	strncpy(name, os->pc, 64);
	os->pc += strlen(name)+1;
	void *fun = (void*)os->ffi.getFun(name);
	if(r1 == 0)
		*(void (*)())fun;
	else if(r1 == 1)
		ffiCallFun1(fun, os);
	else if(r1 == 2)
		ffiCallFun2(fun, os);
	else if(r1 == 3)
		ffiCallFun3(fun, os);
	else
		panic("not yet impl ffiCall>3\n");
	std::cout<<"end fficall\n";
}

void ffiCallFun1(void *fun, OpStream *os)
{
	uint64_t r1 = os->regs[0];
	void (*f1)(uint64_t) = (void (*)(uint64_t))fun;
	f1(r1);
}

void ffiCallFun2(void *fun, OpStream *os)
{
	uint64_t r1 = os->regs[0];
	uint64_t r2 = os->regs[1];
	void (*f1)(uint64_t, uint64_t) = (void (*)(uint64_t, uint64_t))fun;
	f1(r1, r2);
}

void ffiCallFun3(void *fun, OpStream *os)
{
	uint64_t r1 = os->regs[0];
	uint64_t r2 = os->regs[1];
	uint64_t r3 = os->regs[2];
	void (*f1)(uint64_t, uint64_t, uint64_t) = (void (*)(uint64_t, uint64_t, uint64_t))fun;
	f1(r1, r2, r3);
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
	dat2++;
	os->pc = (char*)dat2;
}

void movrFun(OpStream *os)
{
	char r1, r2;
	os->pc = getRegs(os->pc, &r1, &r2);
	os->regs[r1] = os->regs[r2];
}

void cmpFun(OpStream *os)
{
	char r1, r2;
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
	printOps(os);
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

void laFun(OpStream *os)
{
	uint32_t *offset;
	uintptr_t obj;
	offset = (uint32_t*)os->pc;
	obj = (uintptr_t)os->prog + *offset;
	offset++;
	os->pc = (char*)offset;
	os->regs[0] = obj;
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
	(*os)[LA] = laFun;
	(*os)[0] = testOutFun;
}

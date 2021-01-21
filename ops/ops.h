#pragma once

#include <cstdint>
#include <functional>
#include <stack>
#include <map>
#include <list>

#ifdef WITH_FFI
#include "ffi.h"
#endif

enum OPS
{
    ADD = 1,
    SUB,
    MUL,
    DIV,
    XOR,
    AND,
    OR,
    LSHIFT,
    RSHIFT,
    PUSH,
    POP,
    FFIBINDLIB,
    FFIBINDFUN,
    FFICALL,
    JE,
    JNE,
    CMP,
    MOV,
    MOVQ,
    MOVR,
    HLT,
    LA,
    CALL,
    RET
};

class OpStream
{
public:
    #ifdef WITH_FFI
	FFI ffi;
    #endif
    std::map<std::string, std::function<void(OpStream*)>> hostFuns;
    std::map<std::string, uint32_t> childFuns;
    char *pc, *prog, *start;
    std::list<uintptr_t> progStack;
    uintptr_t regs[16];
    size_t len;
    std::function<void(OpStream*)> funs[64];
    void processOp(char op);
    OpStream(uint32_t size, char *ops, uint32_t len);
    ~OpStream();
    void run(OPS);
    void registerFun(std::function<void(OpStream*)> fun, char op);
    void bindHostFun(std::string, std::function<void(OpStream*)> fun);
    void unbindHostFun(std::string);
    void callVMFun(std::string);
    std::function<void(OpStream*)>& operator [](int i)
    {
        return funs[i];
    }
};

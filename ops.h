#pragma once

#include <cstdint>
#include <functional>
#include <stack>

#include "ffi.h"

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
    MOVR,
    HLT,
    LA
};

class OpStream
{
public:
	FFI ffi;
    char *pc, *prog, *start;
    std::stack<uintptr_t> progStack;
    uintptr_t regs[16];
    size_t len;
    std::function<void(OpStream*)> funs[64];
    void processOp(char op);
    OpStream(uint32_t size, char *ops, uint32_t len);
    ~OpStream();
    void run();
    void registerFun(std::function<void(OpStream*)> fun, char op);
    std::function<void(OpStream*)>& operator [](int i)
    {
        return funs[i];
    }
};

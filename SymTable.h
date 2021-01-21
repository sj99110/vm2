#pragma once

#include <map>
#include <tuple>
#include <string>

struct SymHeader
{
    uint32_t MAGIC_NUMBER; // 0x800C244
    char version;
    uint32_t entries;
    uint32_t size;
};

struct SymTableEntry
{
    uint8_t type;
    uint32_t offset;
    uint32_t name;
};

struct SymTable
{
    SymHeader header;
    SymTableEntry entries[];
};

std::pair<uint32_t, std::map<std::string, uint32_t>> buildSymTable(char *pc);
std::pair<std::string, uint32_t> handleEntry(SymTableEntry entry, char *pc);
uint32_t createSymTable(char *pc, std::list<std::pair<uint32_t, uint32_t>> syms);

#include <cstring>
#include <tuple>
#include <exception>


#include "SymTable.h"

std::pair<uint32_t, std::map<std::string, uint32_t>> buildSymTable(char *pc)
{
    SymTable *table = (SymTable*)pc;
    if(table->header.MAGIC_NUMBER != 0x800C244)
        throw std::invalid_argument("Magic number mismatch");
    uint32_t num = table->header.entries;
    SymTableEntry *entries = table->entries;
    uint32_t offset = table->header.size;
    std::map<std::string, uint32_t> funs;
    for(int i=0;i<num;i++)
        funs.insert(handleEntry(entries[i], pc + offset));
    auto ret = std::pair<uint32_t, std::map<std::string, uint32_t>>(offset, funs);
    return ret;
}

std::pair<std::string, uint32_t> handleEntry(SymTableEntry entry, char *pc)
{
    uint8_t type = entry.type;
    uint32_t n = entry.name;
    char *name;
    uint32_t offset = entry.offset;
    uint32_t len = strlen(pc + n);
    name = new char[len];
    strcpy(name, (pc + n));
    std::string _name(name);
    auto ret = std::pair<std::string, uint32_t>(_name, offset);
    delete name;
    return ret;
}

// See LICENSE for license details.

#ifndef _ELFLOADER_H
#define _ELFLOADER_H

#include "elf.h"
#include "memif.h"
#include <map>
#include <stdint.h>
#include <string>

template<typename Word>
struct ElfSection {
    std::string name;
    Word type;
    Word flags;
    Word virt_addr;
    Word file_offset;
    Word size;
    Word link;
    Word info;
    Word addralign;
    Word entsize;
};

template<typename Word>
struct ElfSegment {
    Word type;
    Word file_offset;
    Word virt_addr;
    Word phys_addr;
    Word filesz;
    Word memsz;
    Word flags;
    Word align;
};

template<typename Word>
struct ElfSymbol {
    std::string name;
    Word value;
};

template<typename Word>
struct ElfHeaders {
    std::vector<ElfSection<Word>> sections;
    std::vector<ElfSegment<Word>> segments;
    std::map<std::string, ElfSymbol<Word>> symbols;
};

typedef ElfSection<uint64_t> ElfSection64;
typedef ElfSegment<uint64_t> ElfSegment64;
typedef ElfSymbol<uint64_t> ElfSymbol64;
typedef ElfHeaders<uint64_t> ElfHeaders64;

class memif_t;
ElfHeaders64 load_elf(const char* fn, memif_t* memif, reg_t* entry,
                                         reg_t load_offset, unsigned required_xlen = 0);

#endif

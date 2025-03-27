// See LICENSE for license details.

#include "elfloader.h"
#include "config.h"
#include "elf.h"
#include "memif.h"
#include "byteorder.h"
#include <cstring>
#include <stdint.h>
#include <string>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <assert.h>
#include <unistd.h>
#include <stdexcept>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <map>
#include <cerrno>

struct ByteConverter {
  virtual uint8_t operator()(uint8_t) = 0;
  virtual uint16_t operator()(uint16_t) = 0;
  virtual uint32_t operator()(uint32_t) = 0;
  virtual uint64_t operator()(uint64_t) = 0;
};

struct LeConverter : ByteConverter {
  virtual uint8_t operator()(uint8_t n) { return from_le(n); }
  virtual uint16_t operator()(uint16_t n) { return from_le(n); }
  virtual uint32_t operator()(uint32_t n) { return from_le(n); }
  virtual uint64_t operator()(uint64_t n) { return from_le(n); }
};

struct BeConverter : ByteConverter {
  virtual uint8_t operator()(uint8_t n) { return from_be(n); }
  virtual uint16_t operator()(uint16_t n) { return from_be(n); }
  virtual uint32_t operator()(uint32_t n) { return from_be(n); }
  virtual uint64_t operator()(uint64_t n) { return from_be(n); }
};

template<typename Word, typename ehdr_t, typename phdr_t, typename shdr_t, typename sym_t>
ElfHeaders<Word> LOAD_ELF(
  char* buf, 
  memif_t* memif,
  reg_t* entry,
  reg_t load_offset,
  size_t size,
  ByteConverter& bswap)
{
  ElfHeaders<Word> headers;
  do {
    ehdr_t* eh = (ehdr_t*)buf;
    phdr_t* ph = (phdr_t*)(buf + bswap(eh->e_phoff));
    *entry = bswap(eh->e_entry) + load_offset;
    assert(size >= bswap(eh->e_phoff) + bswap(eh->e_phnum) * sizeof(*ph));
    for (unsigned i = 0; i < bswap(eh->e_phnum); i++) {
      if (bswap(ph[i].p_type) == PT_LOAD && bswap(ph[i].p_memsz)) {
        reg_t load_addr = bswap(ph[i].p_paddr) + load_offset;
        if (bswap(ph[i].p_filesz)) {
          assert(size >= bswap(ph[i].p_offset) + bswap(ph[i].p_filesz));
          memif->write(load_addr, bswap(ph[i].p_filesz),
                       (uint8_t*)buf + bswap(ph[i].p_offset));
        }
        if (size_t pad = bswap(ph[i].p_memsz) - bswap(ph[i].p_filesz)) {
          std::vector<uint8_t> zeros;
          zeros.resize(pad);
          memif->write(load_addr + bswap(ph[i].p_filesz), pad, zeros.data());
        }
      }

      ElfSegment<Word> segment;
      segment.type = bswap(ph[i].p_type);
      segment.file_offset = bswap(ph[i].p_offset);
      segment.virt_addr = bswap(ph[i].p_vaddr);
      segment.phys_addr = bswap(ph[i].p_paddr);
      segment.filesz = bswap(ph[i].p_filesz);
      segment.memsz = bswap(ph[i].p_memsz);
      segment.flags = bswap(ph[i].p_flags);
      segment.align = bswap(ph[i].p_align);
      headers.segments.push_back(segment);
    }
    shdr_t* sh = (shdr_t*)(buf + bswap(eh->e_shoff));
    assert(size >= bswap(eh->e_shoff) + bswap(eh->e_shnum) * sizeof(*sh));
    assert(bswap(eh->e_shstrndx) < bswap(eh->e_shnum));
    assert(size >= bswap(sh[bswap(eh->e_shstrndx)].sh_offset) +
                       bswap(sh[bswap(eh->e_shstrndx)].sh_size));
    char* shstrtab = buf + bswap(sh[bswap(eh->e_shstrndx)].sh_offset);
    unsigned strtabidx = 0, symtabidx = 0;
    for (unsigned i = 0; i < bswap(eh->e_shnum); i++) {
      unsigned max_len =
          bswap(sh[bswap(eh->e_shstrndx)].sh_size) - bswap(sh[i].sh_name);
      assert(bswap(sh[i].sh_name) < bswap(sh[bswap(eh->e_shstrndx)].sh_size));
      assert(strnlen(shstrtab + bswap(sh[i].sh_name), max_len) < max_len);
      if (bswap(sh[i].sh_type) & SHT_NOBITS) continue;
      assert(size >= bswap(sh[i].sh_offset) + bswap(sh[i].sh_size));

      ElfSection<Word> section;
      section.name = shstrtab + bswap(sh[i].sh_name);
      section.type = bswap(sh[i].sh_type);
      section.flags = bswap(sh[i].sh_flags);
      section.virt_addr = bswap(sh[i].sh_addr);
      section.file_offset = bswap(sh[i].sh_offset);
      section.size = bswap(sh[i].sh_size);
      section.link = bswap(sh[i].sh_link);
      section.info = bswap(sh[i].sh_info);
      section.addralign = bswap(sh[i].sh_addralign);
      section.entsize = bswap(sh[i].sh_entsize);
      headers.sections.push_back(section);

      if (section.name == ".strtab")
        strtabidx = i;
      if (section.name == ".symtab")
        symtabidx = i;
    }
    if (strtabidx && symtabidx) {
      char* strtab = buf + bswap(sh[strtabidx].sh_offset);
      sym_t* sym = (sym_t*)(buf + bswap(sh[symtabidx].sh_offset));
      for (unsigned i = 0; i < bswap(sh[symtabidx].sh_size) / sizeof(sym_t);
           i++) {
        unsigned max_len =
            bswap(sh[strtabidx].sh_size) - bswap(sym[i].st_name);
        assert(bswap(sym[i].st_name) < bswap(sh[strtabidx].sh_size));
        assert(strnlen(strtab + bswap(sym[i].st_name), max_len) < max_len);

        ElfSymbol<Word> symbol {
          .name = strtab + bswap(sym[i].st_name), 
          .value = bswap(sym[i].st_value) + load_offset
        };
        headers.symbols[symbol.name] = symbol;
      }
    }
  } while (0);

  return headers;
}

ElfHeaders64 load_elf(const char* fn, memif_t* memif, reg_t* entry,
                                         reg_t load_offset, unsigned required_xlen)
{
  ElfHeaders64 elf;
  int fd = open(fn, O_RDONLY);
  struct stat s;
  if (fd == -1)
      throw std::invalid_argument(std::string("Specified ELF can't be opened: ") + strerror(errno));
  if (fstat(fd, &s) < 0)
    abort();
  size_t size = s.st_size;

  char* buf = (char*)mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (buf == MAP_FAILED)
      throw std::invalid_argument(std::string("Specified ELF can't be mapped: ") + strerror(errno));
  close(fd);

  assert(size >= sizeof(Elf64_Ehdr));
  const Elf64_Ehdr* eh64 = (const Elf64_Ehdr*)buf;
  assert(IS_ELF32(*eh64) || IS_ELF64(*eh64));
  unsigned xlen = IS_ELF32(*eh64) ? 32 : 64;
  if (required_xlen != 0 && required_xlen != xlen) {
    throw incompat_xlen(required_xlen, xlen);
  }
  assert(IS_ELFLE(*eh64) || IS_ELFBE(*eh64));
  assert(IS_ELF_EXEC(*eh64) || IS_ELF_DYN(*eh64));
  assert(IS_ELF_RISCV(*eh64) || IS_ELF_EM_NONE(*eh64));
  assert(IS_ELF_VCURRENT(*eh64));

  if (IS_ELF_EXEC(*eh64)) {
    load_offset = 0;
  }

  std::map<std::string, uint64_t> symbols;

  if (IS_ELFLE(*eh64)) {
    LeConverter bswap;
    if (memif->get_target_endianness() != endianness_little) {
      throw std::invalid_argument("Specified ELF is little endian, but system uses a big-endian memory system. Rerun without --big-endian");
    }
    if (IS_ELF32(*eh64))
      elf = LOAD_ELF<uint64_t, Elf32_Ehdr, Elf32_Phdr, Elf32_Shdr, Elf32_Sym>(
        buf, memif, entry, load_offset, size, bswap
      );
    else
      elf = LOAD_ELF<uint64_t, Elf64_Ehdr, Elf64_Phdr, Elf64_Shdr, Elf64_Sym>(
        buf, memif, entry, load_offset, size, bswap
      );
  } else {
#ifndef RISCV_ENABLE_DUAL_ENDIAN
    throw std::invalid_argument("Specified ELF is big endian.  Configure with --enable-dual-endian to enable support");
#else
    BeConverter bswap;
    if (memif->get_target_endianness() != endianness_big) {
      throw std::invalid_argument("Specified ELF is big endian, but system uses a little-endian memory system. Rerun with --big-endian");
    }
    if (IS_ELF32(*eh64))
      elf = LOAD_ELF<uint64_t, Elf32_Ehdr, Elf32_Phdr, Elf32_Shdr, Elf32_Sym>(
        buf, memif, entry, load_offset, size, bswap
      );
    else
      elf = LOAD_ELF<uint64_t, Elf64_Ehdr, Elf64_Phdr, Elf64_Shdr, Elf64_Sym>(
        buf, memif, entry, load_offset, size, bswap
      );
#endif
  }

  munmap(buf, size);

  return elf;
}

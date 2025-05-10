#ifndef _RISCV_DECODE_MACROS_TYPE_TAG_H
#define _RISCV_DECODE_MACROS_TYPE_TAG_H

#include <typetag/typetag.h>
#include "config.h"
#include "decode_macros.h"

#ifdef TYPE_TAGGING_ENABLED
  #define IF_TAG_PROPAGATION(code) if(p->get_tag_propagation_enabled()) { code }
  #define IF_TAG_CHECKING(code) if(p->get_tag_checking_enabled()) { code }

  #define TAG_ADDR(addr) (p->get_mmu()->translate_tag_addr(addr))
  #define READ_REG_TAG(reg) (CHECK_REG(reg), STATE.XPR_tags[reg])
  #define WRITE_REG_TAG(reg, value) ({ \
      CHECK_REG(reg); \
      typetag_t wdata = (value); /* value may have side effects */ \
      STATE.XPR_tags.write(reg, wdata); \
    })

  // TODO [TAG]: Remove these, they're redundant now
  #define TAG_MMU_STORE(addr, type, tag) MMU.tag_store<type>(addr, tag)
  #define TAG_MMU_LOAD(addr, type) MMU.tag_load<type>(addr)

  #define TAG_TRAP(exception_num, message, exception) \
    if(p->get_tag_trap_mode(exception_num) == TrapMode::TRAP_WARN) { \
      printf("[SPIKE DEBUG] TRAP (warn): " message "\n"); \
    } \
    else if(p->get_tag_trap_mode(exception_num) == TrapMode::TRAP_ENABLED) { \
      printf("[SPIKE DEBUG] TRAP: " message "\n"); \
      throw exception; \
    }

#define TYPE_TAGGING_DEBUG

#else
  #define IF_TAG_PROPAGATION(code)
  #define IF_TAG_CHECKING(code)

#endif // TYPE_TAGGING_ENABLED

#endif // _RISCV_DECODE_MACROS_TYPE_TAG_H
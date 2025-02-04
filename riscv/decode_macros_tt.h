#ifndef _RISCV_DECODE_MACROS_TYPE_TAG_H
#define _RISCV_DECODE_MACROS_TYPE_TAG_H

#include <typetag/typetag.h>
#include "config.h"
#include "decode_macros.h"

#ifdef TYPE_TAGGING_ENABLED
  #define IF_TAG_PROPAGATION(code) if(p->get_tag_propagation_enabled()) { code }
  #define IF_TAG_CHECKING(code) if(p->get_tag_checking_enabled()) { code }

  #define TAG_MMU (*p->get_tag_mmu())
  #define READ_REG_TAG(reg) (CHECK_REG(reg), STATE.XPR_tags[reg])
  #define WRITE_REG_TAG(reg, value) ({ \
      CHECK_REG(reg); \
      typetag_t wdata = (value); /* value may have side effects */ \
      STATE.XPR_tags.write(reg, wdata); \
    })

// #define TYPE_TAGGING_DEBUG

#else
  #define IF_TAG_PROPAGATION(code)
  #define IF_TAG_CHECKING(code)

#endif // TYPE_TAGGING_ENABLED

#endif // _RISCV_DECODE_MACROS_TYPE_TAG_H
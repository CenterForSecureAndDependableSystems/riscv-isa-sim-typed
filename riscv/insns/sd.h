require((xlen == 64) || p->extension_enabled(EXT_ZILSD));

reg_t addr = RS1 + insn.s_imm();
if (xlen == 32) {
  MMU.store<uint64_t>(addr, RS2_PAIR);
} else {
  MMU.store<uint64_t>(addr, RS2);
}

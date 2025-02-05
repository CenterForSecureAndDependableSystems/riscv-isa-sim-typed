#ifdef TYPE_TAGGING_ENABLED
if(insn.rd() == 0 && p->get_tag_propagation_enabled()) {
	// Debug instruction: get a tag
	reg_t addr = RS1 + insn.s_imm();
	WRITE_REG(insn.rs2(), TAG_MMU.load<typetag_t>(addr));
}
else
#endif
	WRITE_RD(sext_xlen(RS1 << (RS2 & (xlen-1))));

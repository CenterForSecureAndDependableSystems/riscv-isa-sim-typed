#ifdef TYPE_TAGGING_ENABLED
if(sreg_t(RD) == 0 && p->get_tag_propagation_enabled()) {
	// Debug instruction: store a tag
	reg_t addr = RS1 + insn.s_imm();
	typetag_t tag = RS2;
	TAG_MMU.store<typetag_t>(addr, tag);
}
else
#endif
	WRITE_RD(sreg_t(RS1) < sreg_t(RS2));

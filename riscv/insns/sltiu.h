#ifdef TYPE_TAGGING_ENABLED
if(insn.rd() == 0) {
	p->set_tag_trap_mode(RS1, (TrapMode)insn.i_imm());

#ifdef TYPE_TAGGING_DEBUG
	if(RS2) {
		printf("[SPIKE DEBUG] Enabled tag exception %ld\n", RS1);
	}
	else {
		printf("[SPIKE DEBUG] Disabled tag exception %ld\n", RS1);
	}
#endif
}
#endif

WRITE_RD(RS1 < reg_t(insn.i_imm()));

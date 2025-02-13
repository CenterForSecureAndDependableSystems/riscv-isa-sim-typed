CHECK_RD();
reg_t tmp = npc;
set_pc(JUMP_TARGET);
WRITE_RD(tmp);

// Tag return address 
#ifdef TYPE_TAGGING_ENABLED
if(p->get_tag_propagation_enabled()) {
	// Ignore indirect jumps for now
	if(insn.rd() != 0) {
		WRITE_REG_TAG(insn.rd(), tt_set_obj_type(0, TT_OBJ_RETURN));
	}
}
#endif

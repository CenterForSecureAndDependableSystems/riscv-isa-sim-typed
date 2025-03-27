CHECK_RD();
reg_t tmp = npc;
set_pc(JUMP_TARGET);
WRITE_RD(tmp);

// Tag return address 
#ifdef TYPE_TAGGING_ENABLED
if(p->get_tag_propagation_enabled()) {
	if(insn.rd() != 0) {
		typetag_t tag = tt_set_obj_type(0, TT_OBJ_RETURN);
		WRITE_REG_TAG(insn.rd(), tag);
	}
}
#endif // TYPE_TAGGING_ENABLED

CHECK_RD();
reg_t tmp = npc;
set_pc(JUMP_TARGET);
WRITE_RD(tmp);

// Tag return address 
IF_TAG_PROPAGATION(
	typetag_t tag = tt_set_obj_type(0, TT_OBJ_RETURN);
	WRITE_REG_TAG(insn.rd(), tag);
)

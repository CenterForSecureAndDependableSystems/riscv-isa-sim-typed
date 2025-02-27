CHECK_RD();


#ifdef TYPE_TAGGING_ENABLED
if(p->get_tag_checking_enabled()) {
    // If this is a `ret` pseudoinstruction (RD == x0)
    // and the tag is not marked as a valid return address,
    // panic.
    if(insn.rd() == 0 && insn.rs1() != 0 && tt_get_obj_type(READ_REG_TAG(insn.rs1())) != TT_OBJ_RETURN) {
        TAG_TRAP(TT_EXP_INVALID_RETURN_TAG, "Return address was not tagged appropriately", trap_return_tag_fault());
    }
}
#endif

reg_t tmp = npc;
set_pc((RS1 + insn.i_imm()) & ~reg_t(1));
WRITE_RD(tmp);

// Tag return address 
#ifdef TYPE_TAGGING_ENABLED
if(p->get_tag_propagation_enabled()) {
	typetag_t tag = 0;
	tag = tt_set_obj_type(tag, TT_OBJ_RETURN);
	WRITE_REG_TAG(insn.rd(), tag);
}
#endif

if (ZICFILP_xLPE(STATE.v, STATE.prv)) {
    STATE.elp = ZICFILP_IS_LP_EXPECTED(insn.rs1());
    serialize();
}

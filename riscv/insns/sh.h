reg_t addr = READ_REG(insn.rs1()) + insn.s_imm();
MMU.store<uint16_t>(addr, RS2);

IF_TAG_PROPAGATION(
	TAG_MMU_STORE(addr, uint16_t, READ_REG_TAG(insn.rs2()));
)
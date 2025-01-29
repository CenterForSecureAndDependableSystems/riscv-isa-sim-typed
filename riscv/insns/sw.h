// MMU.store<uint32_t>(RS1 + insn.s_imm(), RS2);
reg_t addr = READ_REG(insn.rs1()) + insn.s_imm();
MMU.store<uint32_t>(addr, READ_REG(insn.rs2()));

IF_TAG_PROPAGATION(
	TAG_MMU.store<uint32_t>(addr, READ_REG_TAG(insn.rs2()));
)
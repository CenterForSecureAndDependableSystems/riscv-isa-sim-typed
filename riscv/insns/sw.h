// MMU.store<uint32_t>(RS1 + insn.s_imm(), RS2);
reg_t addr = RS1 + insn.s_imm();
MMU.store<uint32_t>(addr, RS2);

IF_TAG_PROPAGATION(
	TAG_MMU.store<raw_tag_t>(addr, READ_REG_TAG(insn.rs2()));
)
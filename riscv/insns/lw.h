reg_t addr = RS1 + insn.i_imm();
WRITE_RD(MMU.load<int32_t>(addr));

IF_TAG_PROPAGATION(
	WRITE_REG_TAG(insn.rd(), TAG_MMU_LOAD(addr, int32_t));
)
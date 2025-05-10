require_extension(EXT_ZALASR);
WRITE_RD(MMU.load<int32_t>(RS1));

IF_TAG_PROPAGATION(
	WRITE_REG_TAG(insn.rd(), TAG_MMU_LOAD(RS1, int32_t));
)
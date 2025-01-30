require_extension(EXT_ZALASR);
WRITE_RD(MMU.load<uint8_t>(RS1));

IF_TAG_PROPAGATION(
	WRITE_REG_TAG(insn.rd(), TAG_MMU.load<uint8_t>(RS1));
)
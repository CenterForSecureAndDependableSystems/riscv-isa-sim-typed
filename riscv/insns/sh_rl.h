require_extension(EXT_ZALASR);
MMU.store<uint16_t>(RS1, RS2);

IF_TAG_PROPAGATION(
	TAG_MMU.store<uint16_t>(RS1, READ_REG_TAG(insn.rs2()));
)
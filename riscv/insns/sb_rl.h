require_extension(EXT_ZALASR);
MMU.store<uint8_t>(RS1, RS2);

IF_TAG_PROPAGATION(
	TAG_MMU_STORE(RS1, uint8_t, READ_REG_TAG(insn.rs2()));
)
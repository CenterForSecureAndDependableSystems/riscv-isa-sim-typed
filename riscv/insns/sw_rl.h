require_extension(EXT_ZALASR);
MMU.store<uint32_t>(RS1, RS2);

IF_TAG_PROPAGATION(
	TAG_MMU_STORE(RS1, uint32_t, READ_REG_TAG(insn.rs2()));
)
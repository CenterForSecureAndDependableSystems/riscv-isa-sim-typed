require_rv64;
require_extension(EXT_ZALASR);
MMU.store<uint64_t>(RS1, RS2);

IF_TAG_PROPAGATION(
	TAG_MMU_STORE(RS1, uint64_t, READ_REG_TAG(insn.rs2()));
)
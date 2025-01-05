#ifdef TYPE_TAGGING_ENABLED
if(sreg_t(RD) == 0) {
	switch(sreg_t(insn.i_imm())) {
	case 2:
		p->set_tag_checking(false);
		printf("[SPIKE DEBUG] Tag checking disabled\n");
		break;
	case 3:
		p->set_tag_checking(true);
		printf("[SPIKE DEBUG] Tag checking enabled\n");
		break;
	}
}
else {
#endif

WRITE_RD(sreg_t(RS1) < sreg_t(insn.i_imm()));

#ifdef TYPE_TAGGING_ENABLED
}
#endif

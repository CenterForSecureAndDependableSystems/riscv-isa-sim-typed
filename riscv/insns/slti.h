#ifdef TYPE_TAGGING_ENABLED
if(insn.rd() == 0) {
	switch(sreg_t(insn.i_imm())) {
	case 0:
		p->set_tag_propagation(false);
#ifdef TYPE_TAGGING_DEBUG
		printf("[SPIKE DEBUG] Tag propagation disabled\n");
#endif
		break;
	case 1:
		p->set_tag_propagation(true);
#ifdef TYPE_TAGGING_DEBUG
		printf("[SPIKE DEBUG] Tag propagation enabled\n");
#endif
		break;
	case 2:
		p->set_tag_checking(false);
#ifdef TYPE_TAGGING_DEBUG
		printf("[SPIKE DEBUG] Tag checking disabled\n");
#endif
		break;
	case 3:
		p->set_tag_checking(true);
#ifdef TYPE_TAGGING_DEBUG
		printf("[SPIKE DEBUG] Tag checking enabled\n");
#endif
		break;
	}
}
else {
#endif

	WRITE_RD(sreg_t(RS1) < sreg_t(insn.i_imm()));

#ifdef TYPE_TAGGING_ENABLED
}
#endif


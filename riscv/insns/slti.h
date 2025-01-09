#ifdef TYPE_TAGGING_ENABLED
if(sreg_t(RD) == 0) {
	switch(sreg_t(insn.i_imm())) {
	case 0:
		p->set_tag_propagation(false);
		printf("[SPIKE DEBUG] Tag propagation disabled\n");
		break;
	case 1:
		p->set_tag_propagation(true);
		printf("[SPIKE DEBUG] Tag propagation enabled\n");
		break;
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
#endif

WRITE_RD(sreg_t(RS1) < sreg_t(insn.i_imm()));

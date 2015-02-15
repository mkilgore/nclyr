
objs-y += config_parser.tab.o
objs-y += config.o
objs-y += config_c.o
objs-y += config_m.o
objs-y += config_lex.o

$(srctree)/config.o: $(srctree)/config_parser.tab.h
$(srctree)/config_lex.o: $(srctree)/config_parser.tab.h

$(srctree)/%.tab.c $(srctree)/%.tab.h: $(srctree)/%.y
	@$(call mecho," YACC    $(dir $@)$*.tab.c $(dir $@)$*.tab.h","$(YACC) $(YFLAGS) -dtv $<")
	$(Q)$(YACC) $(YFLAGS) -d -b $(dir $<)/$* $<

$(srctree)/config_lex.c: $(srctree)/config_lex.l
	@$(call mecho," LEX     $@","$(LEX) $(LFLAGS) -o $@ $<")
	$(Q)$(LEX) $(LFLAGS) -o $@ $<

CLEAN_LIST += $(srctree)/config_lex.c $(srctree)/config_parser.tab.c $(srctree)/config_parser.tab.h
.SECONDARY: $(srctree)/config_parser.tab.c $(srctree)/config_parser.tab.h


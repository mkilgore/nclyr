
objs-y += config_lexer.o
objs-y += config_parser.tab.o

clean-list-y += config_lexer.c

$(srctree)/%.tab.c $(srctree)/%.tab.h: $(srctree)/%.y
	@$(call mecho," YACC    $(dir $@)$*.tab.c $(dir $@)$*.tab.h","$(YACC) $(YFLAGS) -dtv $<")
	$(Q)$(YACC) $(YFLAGS) -d -b $(dir $<)/$* $<

$(objtree)/config_lexer.c: $(srctree)/config_lexer.l
	@$(call mecho," LEX     $@","$(LEX) $(LFLAGS) -o $@ $<")
	$(Q)$(LEX) $(LFLAGS) -o $@ $<

clean-list-y += $(srctree)/config_parser.tab.c $(srctree)/config_parser.tab.h
.SECONDARY: $(srctree)/config_parser.tab.c $(srctree)/config_parser.tab.h


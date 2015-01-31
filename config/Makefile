
objs-y += config.o
objs-y += config_c.o
objs-y += config_m.o
objs-y += config_lex.o

$(objtree)/config_lex.c: $(srctree)/config_lex.l
	@$(call mecho," LEX     $@","$(LEX) $(LFLAGS) -o $@ $<")
	$(Q)$(LEX) $(LFLAGS) -o $@ $<

CLEAN_LIST += $(objtree)/config_lex.c


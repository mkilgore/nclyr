
objs-y += config.o
objs-y += config_c.o
objs-y += config_m.o
objs-y += config_lex.o

$(srctree)/config_lex.c: $(srctree)/config_lex.l
	@$(call mecho," LEX     $@","$(LEX) $(LFLAGS) -o $@ $<")
	$(Q)$(LEX) $(LFLAGS) -o $@ $<

CLEAN_LIST += $(srctree)/config_lex.c



objs-y += config_lexer.o
objs-y += config_color.o
objs-y += config_file.o
objs-y += config_help.o
objs-y += config_item.o
objs-y += config_print.o
objs-y += config_arg.o

clean-list-y += config_lexer.c

$(objtree)/config_lexer.c: $(srctree)/config_lexer.l
	@$(call mecho," LEX     $@","$(LEX) $(LFLAGS) -o $@ $<")
	$(Q)$(LEX) $(LFLAGS) -o $@ $<

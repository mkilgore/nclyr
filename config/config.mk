# Program wide settings
EXE       := config
EXEC      := NCLYR_CONFIG
NCLYR_CONFIG_VERSION   := 0
NCLYR_CONFIG_SUBLEVEL  := 1
NCLYR_CONFIG_PATCH     := 0
NCLYR_CONFIG_VERSION_N := $(NCLYR_CONFIG_VERSION).$(NCLYR_CONFIG_SUBLEVEL).$(NCLYR_CONFIG_PATCH)

NCLYR_CONFIG_LIBFLAGS :=
NCLYR_CONFIG_CFLAGS  += -DNCLYR_CONFIG_VERSION=$(NCLYR_CONFIG_VERSION)       \
				 -DNCLYR_CONFIG_SUBLEVEL=$(NCLYR_CONFIG_SUBLEVEL)     \
				 -DNCLYR_CONFIG_PATCH=$(NCLYR_CONFIG_PATCH)           \
				 -DNCLYR_CONFIG_VERSION_N="$(NCLYR_CONFIG_VERSION_N)"

NCLYR_CONFIG_OBJS += ./config.o

$(objtree)/include/gen_config.h: $(srctree)/nclyr_build.conf $(objtree)/bin/config
	@$(call mecho," CONFIG  $@","./bin/config c < $< > $@")
	$(Q)./bin/config c < $< > $@

$(objtree)/gen_config.mk: $(srctree)/nclyr_build.conf $(objtree)/bin/config
	@$(call mecho," CONFIG  $@","./bin/config m < $< > $@")
	$(Q)./bin/config m < $< > $@

CLEAN_LIST += $(objtree)/include/gen_config.h
CLEAN_LIST += $(objtree)/gen_config.mk


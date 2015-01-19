# Program wide settings
EXE       := nclyr
EXEC      := NCLYR
NCLYR_VERSION   := 0
NCLYR_SUBLEVEL  := 1
NCLYR_PATCH     := 0
NCLYR_VERSION_N := $(NCLYR_VERSION).$(NCLYR_SUBLEVEL).$(NCLYR_PATCH)

NCLYR_LIBFLAGS := -lncurses -lglyr -pthread
NCLYR_CFLAGS  += -DNCLYR_VERSION=$(NCLYR_VERSION)       \
				 -DNCLYR_SUBLEVEL=$(NCLYR_SUBLEVEL)     \
				 -DNCLYR_PATCH=$(NCLYR_PATCH)           \
				 -DNCLYR_VERSION_N="$(NCLYR_VERSION_N)"

NCLYR_OBJS += ./nclyr.o

ifeq ($(CONFIG_PLAYER_MPRIS2),y)
NCLYR_CFLAGS += $(shell pkg-config --cflags dbus-1)
NCLYR_LIBFLAGS += $(shell pkg-config --libs dbus-1)
endif


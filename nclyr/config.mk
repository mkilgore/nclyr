# Program wide settings
EXE       := nclyr
EXEC      := NCLYR
NCLYR_VERSION   := 0
NCLYR_SUBLEVEL  := 9
NCLYR_PATCH     := 0
NCLYR_VERSION_N := $(NCLYR_VERSION).$(NCLYR_SUBLEVEL).$(NCLYR_PATCH)

NCLYR_LIBFLAGS := -pthread
NCLYR_CFLAGS  += -I'./include'                          \
                 -DNCLYR_VERSION=$(NCLYR_VERSION)       \
				 -DNCLYR_SUBLEVEL=$(NCLYR_SUBLEVEL)     \
				 -DNCLYR_PATCH=$(NCLYR_PATCH)           \
				 -DNCLYR_VERSION_N="$(NCLYR_VERSION_N)"

NCLYR_OBJS += ./nclyr.o

ifdef CONFIG_TUI
	NCLYR_LIBFLAGS += -lncurses
endif

ifdef CONFIG_LIB_GLYR
	NCLYR_LIBFLAGS += -lglyr
endif

ifdef CONFIG_PLAYER_MPRIS2
	NCLYR_CFLAGS += $(shell pkg-config --cflags dbus-1)
	NCLYR_LIBFLAGS += $(shell pkg-config --libs dbus-1)
endif

ifdef CONFIG_PLAYER_MPD
	NCLYR_CFLAGS += $(shell pkg-config --cflags libmpdclient)
	NCLYR_LIBFLAGS += $(shell pkg-config --libs libmpdclient)

ifdef CONFIG_TUI_MPD_VISUALIZER
	NCLYR_LIBFLAGS += -lm -lfftw3
endif

endif


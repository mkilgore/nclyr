# Program wide settings
EXE       := nclyr
EXEC      := NCLYR
NCLYR_VERSION   := 0
NCLYR_SUBLEVEL  := 1
NCLYR_PATCH     := 0
NCLYR_VERSION_N := $(NCLYR_VERSION).$(NCLYR_SUBLEVEL).$(NCLYR_PATCH)

NCLYR_LIBFLAGS := -pthread
NCLYR_CFLAGS  += -DNCLYR_VERSION=$(NCLYR_VERSION)       \
				 -DNCLYR_SUBLEVEL=$(NCLYR_SUBLEVEL)     \
				 -DNCLYR_PATCH=$(NCLYR_PATCH)           \
				 -DNCLYR_VERSION_N="$(NCLYR_VERSION_N)"

NCLYR_OBJS += ./nclyr.o

ifeq ($(CONFIG_TUI),y)
NCLYR_LIBFLAGS := -lncurses
endif

ifeq ($(CONFIG_LIB_GLYR),y)
NCLYR_LIBFLAGS += -lglyr
endif

ifeq ($(CONFIG_PLAYER_MPRIS2),y)
NCLYR_CFLAGS += $(shell pkg-config --cflags dbus-1)
NCLYR_LIBFLAGS += $(shell pkg-config --libs dbus-1)
endif

ifeq ($(CONFIG_PLAYER_MPD),y)
NCLYR_CFLAGS += $(shell pkg-config --cflags libmpdclient)
NCLYR_LIBFLAGS += $(shell pkg-config --libs libmpdclient)
endif

NCLYR_BUILD_SETTINGS :=

ifeq ($(CONFIG_PLAYER_MPD),y)
	NCLYR_BUILD_SETTINGS += mpd
endif
ifeq ($(CONFIG_PLAYER_PIANOBAR),y)
	NCLYR_BUILD_SETTINGS += pianobar
endif
ifeq ($(CONFIG_LIB_GLYR),y)
	NCLYR_BUILD_SETTINGS += glyr
endif
ifeq ($(CONFIG_TUI),y)
	NCLYR_BUILD_SETTINGS += tui
endif

NCLYR_CFLAGS += -DNCLYR_BUILD_SETTINGS="$(NCLYR_BUILD_SETTINGS)"


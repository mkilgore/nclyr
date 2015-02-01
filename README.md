nclyr
=====

`nclyr` is a music player client, intended to be a front-end to a variety of
music players and music player protocols.

Currently supported players/protocols:

    mpd
    pianobar (not fully supported)

`nclyr` also has the ability of displaying music information, currently via the
[`glyr`](https://github.com/sahib/glyr) library.

Building
========

`nclyr` is built using `make`. `make` or `make all` will build the entire
project, with `./bin/nclyr` being created. `make clean` will remove the `./bin`
directory, as well as remove any object and dependency files.

Configuration is done via editing the "./nclyr_build.conf" file before running
'make'.  Current build configuration options are:

    CONFIG_LIB_GLYR - If not set to 'y', GLYR will not be linked and any
                      functionality requiring `glyr` will not be compiled in.

These configuration options select the currently supported players, set to 'y'
to compile in support, set to 'n' to disable:

    CONFIG_PLAYER_PIANOBAR - Pulls no extra dependencies.
    CONFIG_PLAYER_MPD - Pulls `libmpdclient`.

These configuration options select interfaces to compile. Set to 'y' to compile
that interface in, set 'n' to disable.

    CONFIG_TUI - Pulls `ncurses`
    CONFIG_CONSOLE - Pulls no extra dependencies

These configuration options set the defaults to use in the program:

    CONFIG_DEFAULT_IFACE - Set to a string that is the name of the interface to
                           use when no option is supplied to `nclyr`.
    CONFIG_DEFAULT_PLAYER - Set to a string that is the name of the player to
                            use when no player is supplied to `nclyr`.


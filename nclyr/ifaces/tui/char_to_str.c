
#include "common.h"

#include <ncurses.h>

#include "char_to_str.h"

static const char *key_chart[] = {
    ['\n'] = "Enter",
    [' '] = "Space",
    "!", "\"", "#", "$", "%", "&", "'", "(", ")", "*",
    "+", ",", "-", ".", "/", "0", "1", "2", "3", "4",
    "5", "6", "7", "8", "9", ":", ";", "<", "=", ">",
    "?", "@", "A", "B", "C", "D", "E", "F", "G", "H",
    "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R",
    "S", "T", "U", "V", "W", "X", "Y", "Z", "[", "\\",
    "]", "^", "_", "`", "a", "b", "c", "d", "e", "f",
    "g", "h", "i", "j", "k", "l", "m", "n", "o", "p",
    "q", "r", "s", "t", "u", "v", "w", "x", "y", "z",
    "{", "}", "~",

    [KEY_BREAK] = "break",
    [KEY_DOWN] = "Arrow Down",
    [KEY_UP] = "Arrow Up",
    [KEY_LEFT] = "Arrow Left",
    [KEY_RIGHT] = "Arrow Right",
    [KEY_HOME] = "Home",
    [KEY_BACKSPACE] = "Backspace",
    [KEY_F0] = "F0",
    [KEY_F(1)] = "F1",
    [KEY_F(2)] = "F2",
    [KEY_F(3)] = "F3",
    [KEY_F(4)] = "F4",
    [KEY_F(5)] = "F5",
    [KEY_F(6)] = "F6",
    [KEY_F(7)] = "F7",
    [KEY_F(8)] = "F8",
    [KEY_F(9)] = "F9",
    [KEY_F(10)] = "F10",
    [KEY_F(11)] = "F11",
    [KEY_F(12)] = "F12",
    [KEY_END] = "End",
    [KEY_NPAGE] = "Page Down",
    [KEY_PPAGE] = "Page Up",
};

const char *char_to_str(int ch)
{
     return key_chart[ch];
}

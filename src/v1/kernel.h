#ifndef KERNEL_H
#define KERNEL_H

#include "types.h"

#define NULL 0

#define VGA_ADDRESS 0xB8000
#define BUFSIZE 2200

uint16 *vga_buffer;

#define BOX_MAX_WIDTH 78
#define BOX_MAX_HEIGHT 23

#define BOX_SINGLELINE 1
#define BOX_DOUBLELINE 2

#define VGA_ADDRESS 0xB8000
#define VGA_MAX 0xF9FF
#define VGA_MAX_WIDTH 320
#define VGA_MAX_HEIGHT 200
#define CHAR_UP 30
#define CHAR_DOWN 17
#define CHAR_RIGHT 16
#define CHAR_LEFT 31

enum vga_color
{
    BLACK,
    BLUE,
    GREEN,
    CYAN,
    RED,
    MAGENTA,
    BROWN,
    GREY,
    DARK_GREY,
    BRIGHT_BLUE,
    BRIGHT_GREEN,
    BRIGHT_CYAN,
    BRIGHT_RED,
    BRIGHT_MAGENTA,
    YELLOW,
    WHITE,
};

/* Attribute Controller Registers */
#define VGA_AC_INDEX 0x3C0
#define VGA_AC_READ 0x3C1
#define VGA_AC_WRITE 0x3C0

/*
Miscellaneous Output
*/
#define VGA_MISC_READ 0x3CC
#define VGA_MISC_WRITE 0x3C2

/* Sequencer Registers */
#define VGA_SEQ_INDEX 0x3C4
#define VGA_SEQ_DATA 0x3C5

/* VGA Color Palette Registers */
#define VGA_DAC_READ_INDEX 0x3C7
#define VGA_DAC_WRITE_INDEX 0x3C8
#define VGA_DAC_DATA 0x3C9

/* Graphics Controller Registers */
#define VGA_GC_INDEX 0x3CE
#define VGA_GC_DATA 0x3CF

/* CRT Controller Registers */
#define VGA_CRTC_INDEX 0x3D4
#define VGA_CRTC_DATA 0x3D5

/* General Control and Status Registers */
#define VGA_INSTAT_READ 0x3DA
/* Strings, messages and OS information like OS version and OS name */
#define OS_NAME "Cadex Haltic Fox"
#define OS_VERSION "v1.3.2 Unofficial Build. Non-Linux based."

#include "keyboard.h"

#endif

.include "advanced_common.inc"

.macro TEST_SETUP
    ADVANCED_SETUP
.endm

GX_PROGRAM
ADVANCED_STATE 2, 1
CP_LOAD 0x50, 0x00002A00
ADVANCED_TEXTURES
TEV_ORDER0
TEV_MODULATE

.byte 0x10
.long 0x0003100A
.long 0x202020FF, 0xFFFFFFFF, 0x101040FF, 0xFFFFFFFF
.byte 0x10
.long 0x000F0600
.long 0xFFFFFFFF, 0x3F800000, 0, 0, 0, 0x3F800000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0

.if CASE == 1
    .byte 0x10
    .long 0x0000100E
    .long 0x00000000
.elseif CASE == 2
    .byte 0x10
    .long 0x0000100E
    .long 0x00000403
.elseif CASE == 3
    .byte 0x10
    .long 0x0000100E
    .long 0x00001403
.elseif CASE == 4
    .byte 0x10
    .long 0x0000100E
    .long 0x00002403
.elseif CASE == 5
    .byte 0x10
    .long 0x0000100E
    .long 0x0000040B
    .byte 0x10
    .long 0x000F0610
    .long 0x4040FFFF, 0x3F800000, 0, 0, 0, 0x3F800000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
.elseif CASE == 6
    .byte 0x10
    .long 0x0000100F
    .long 0x00000403
.elseif CASE == 7
    TEV_ORDER0 0, 0, 1
    .byte 0x10
    .long 0x0000100F
    .long 0x00000403
.elseif CASE == 8
    .byte 0x10
    .long 0x00080120
    .long 0, 0xBF800000, 0, 0x3F800000, 0, 0, 0, 0, 0x3F800000, 0
    .byte 0x10
    .long 0x0000100E
    .long 0x00000403
.elseif CASE == 9
    .byte 0x10
    .long 0x0000100E
    .long 0x000004FF
.elseif CASE == 10
    .byte 0x10
    .long 0x0000100E
    .long 0x00000001
.endif

.byte 0x80
.short 4
.long 0xBF400000, 0xBF400000, 0, 0, 0, 0x3F800000, 0xFF6060D0, 0, 0x3F800000, 0x40000000, 0
.long 0x3F400000, 0xBF400000, 0, 0x3F000000, 0, 0x3F5DB3D7, 0x60FF60D0, 0x3F800000, 0x3F800000, 0, 0x40000000
.long 0x3F400000, 0x3F400000, 0, 0xBF000000, 0, 0x3F5DB3D7, 0x6060FFD0, 0x3F800000, 0, 0xC0000000, 0
.long 0xBF400000, 0x3F400000, 0, 0, 0, 0x3F800000, 0xFFFFFFFF, 0, 0, 0, 0xC0000000
GX_END
ADVANCED_DATA

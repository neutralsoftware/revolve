.include "advanced_common.inc"

.macro TEST_SETUP
    ADVANCED_SETUP
.endm

GX_PROGRAM
ADVANCED_STATE 2, 1, 2
CP_LOAD 0x50, 0x0000AA00
CP_LOAD 0x70, 0x01355009
ADVANCED_TEXTURES
TEV_ORDER0
TEV_MODULATE

.byte 0x10
.long 0x00001009
.long 2
.byte 0x10
.long 0x0003100A
.long 0x202020FF, 0xFFFFFFFF, 0x101040FF, 0xFFFFFFFF
.byte 0x10
.long 0x000F0600
.long 0, 0, 0, 0xFFFFFFFF
.long 0x3F800000, 0, 0, 0x3F800000, 0, 0
.long 0, 0, 0x40000000, 0, 0, 0x3F800000

.if CASE == 1
    .byte 0x10
    .long 0x0000100A
    .long 0x808080FF
    .byte 0x10
    .long 0x0000100C
    .long 0xC080FFFF
    .byte 0x10
    .long 0x0000100E
    .long 0x00000402
.elseif CASE == 2
    .byte 0x10
    .long 0x0000100E
    .long 0x00000507
.elseif CASE == 3
    .byte 0x10
    .long 0x00020604
    .long 0xC0400000, 0x40800000, 0
    .byte 0x10
    .long 0x0000100E
    .long 0x00000707
.elseif CASE == 4
    .byte 0x10
    .long 0x00020607
    .long 0x3F800000, 0, 0x3F800000
    .byte 0x10
    .long 0x0000100E
    .long 0x00000707
.elseif CASE == 5
    .byte 0x10
    .long 0x00000603
    .long 0xFF4040FF
    .byte 0x10
    .long 0x0000100E
    .long 0x0000050F
    .byte 0x10
    .long 0x000F0610
    .long 0, 0, 0, 0x4040FFFF
    .long 0x3F800000, 0, 0, 0x3F800000, 0, 0
    .long 0, 0, 0x40000000, 0, 0, 0x3F800000
.elseif CASE == 6
    TEV_ORDER0 0, 0, 1
    .byte 0x10
    .long 0x00020604
    .long 0, 0, 0x3F800000
    .byte 0x10
    .long 0x0000100F
    .long 0x00000207
.elseif CASE == 7
    TEV_ORDER0 0, 0, 1
    .byte 0x10
    .long 0x0000100F
    .long 0x00000507
.elseif CASE == 8
    .byte 0x10
    .long 0x00080400
    .long 0, 0xBF800000, 0, 0x3F800000, 0, 0, 0, 0, 0x3F800000, 0
    .byte 0x10
    .long 0x0000100E
    .long 0x00000507
.elseif CASE == 9
    .byte 0x10
    .long 0x000F0610
    .long 0, 0, 0, 0xFFFFFFFF
    .long 0x3F800000, 0, 0, 0x3F800000, 0, 0
    .long 0, 0, 0x40000000, 0, 0, 0x3F800000
    .byte 0x10
    .long 0x0000100E
    .long 0x0000050F
.elseif CASE == 10
    .byte 0x10
    .long 0x0000100E
    .long 0x00000001
.endif

.byte 0x80
.short 4
.long 0xBF400000, 0xBF400000, 0, 0, 0, 0x3F800000, 0xFF6060D0, 0x40FFFFFF, 0, 0x3F800000, 0x40000000, 0
.long 0x3F400000, 0xBF400000, 0, 0x3F000000, 0, 0x3F5DB3D7, 0x60FF60D0, 0xFF40FFFF, 0x3F800000, 0x3F800000, 0, 0x40000000
.long 0x3F400000, 0x3F400000, 0, 0xBF000000, 0, 0x3F5DB3D7, 0x6060FFD0, 0xFFFF40FF, 0x3F800000, 0, 0xC0000000, 0
.long 0xBF400000, 0x3F400000, 0, 0, 0, 0x3F800000, 0xFFFFFFFF, 0x808080FF, 0, 0, 0, 0xC0000000
GX_END
ADVANCED_DATA

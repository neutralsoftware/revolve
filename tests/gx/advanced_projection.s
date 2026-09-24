.include "advanced_common.inc"

.macro TEST_SETUP
    ADVANCED_SETUP
.endm

GX_PROGRAM
ADVANCED_STATE 2, 2
ADVANCED_TEXTURES
TEV_ORDER01
TEV_REPLACE
BP_LOAD 0xC2, 0x08F8AF
BP_LOAD 0xC3, 0x08F2F0

.if CASE == 1
    .byte 0x10
    .long 0x00001040
    .long 0x00000004
.elseif CASE == 2
    CP_LOAD 0x50, 0x00002A00
    CP_LOAD 0x70, 0x01215009
    .byte 0x10
    .long 0x00001040
    .long 0x00000084
    .byte 0x10
    .long 0x000B0078
    .long 0x3F000000, 0, 0, 0x3F000000
    .long 0, 0x3F000000, 0, 0x3F000000
    .long 0, 0, 0x3F800000, 0
    CP_LOAD 0x30, 0x00000780
.elseif CASE == 3
    .byte 0x10
    .long 0x00001040
    .long 0x00000006
    .byte 0x10
    .long 0x000B0078
    .long 0x3F000000, 0, 0, 0x3F000000
    .long 0, 0x3F000000, 0, 0x3F000000
    .long 0, 0, 0x3F000000, 0x3F800000
    CP_LOAD 0x30, 0x00000780
.elseif CASE == 4
    .byte 0x10
    .long 0x000B0078
    .long 0x3F000000, 0, 0, 0x3E800000
    .long 0, 0x3F000000, 0, 0x3E800000
    .long 0, 0, 0x3F800000, 0
    CP_LOAD 0x30, 0x00000780
.elseif CASE == 5
    .byte 0x10
    .long 0x00001012
    .long 1
    .byte 0x10
    .long 0x00001050
    .long 0
    .byte 0x10
    .long 0x000B0500
    .long 0, 0xBF800000, 0, 0x3F800000
    .long 0x3F800000, 0, 0, 0
    .long 0, 0, 0x3F800000, 0
.elseif CASE == 6
    .byte 0x10
    .long 0x00001012
    .long 1
    .byte 0x10
    .long 0x00001050
    .long 0x00000100
    POST_MATRIX_IDENTITY
.elseif CASE == 7
    .byte 0x10
    .long 0x00001040
    .long 0x00000002
.elseif CASE == 8
    .byte 0x10
    .long 0x00011040
    .long 0x00000002, 0x00000280
.elseif CASE == 9
    CP_LOAD 0x50, 0x00002202
    .byte 0x10
    .long 0x000B0078
    .long 0x3F000000, 0, 0, 0, 0, 0x3F000000, 0, 0, 0, 0, 0x3F800000, 0
.elseif CASE == 10
    .byte 0x10
    .long 0x00011040
    .long 0x00000280, 0x00000380
.endif

.if CASE == 9
    .byte 0x80
    .short 4
    .byte 0x1E
    .long 0xBF400000, 0xBF400000, 0, 0xFF6060D0, 0, 0x3F800000, 0x40000000, 0
    .byte 0
    .long 0x3F400000, 0xBF400000, 0, 0x60FF60D0, 0x3F800000, 0x3F800000, 0, 0x40000000
    .byte 0x1E
    .long 0x3F400000, 0x3F400000, 0, 0x6060FFD0, 0x3F800000, 0, 0xC0000000, 0
    .byte 0
    .long 0xBF400000, 0x3F400000, 0, 0xFFFFFFFF, 0, 0, 0, 0xC0000000
.elseif CASE == 2
    .byte 0x80
    .short 4
    .long 0xBF400000, 0xBF400000, 0, 0xBF000000, 0, 0x3F5DB3D7, 0xFF6060D0, 0, 0x3F800000, 0x40000000, 0
    .long 0x3F400000, 0xBF400000, 0, 0x3F000000, 0, 0x3F5DB3D7, 0x60FF60D0, 0x3F800000, 0x3F800000, 0, 0x40000000
    .long 0x3F400000, 0x3F400000, 0, 0x3F000000, 0, 0x3F5DB3D7, 0x6060FFD0, 0x3F800000, 0, 0xC0000000, 0
    .long 0xBF400000, 0x3F400000, 0, 0xBF000000, 0, 0x3F5DB3D7, 0xFFFFFFFF, 0, 0, 0, 0xC0000000
.else
    DRAW_ADVANCED_QUAD
.endif
GX_END
ADVANCED_DATA

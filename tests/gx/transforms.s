.include "common.inc"

.macro TEST_SETUP
.endm

GX_PROGRAM
CP_LOAD 0x30, 0
CP_LOAD 0x50, 0x00002200
CP_LOAD 0x60, 0
CP_LOAD 0x70, 0x00014009

.if CASE == 1
    XF_IDENTITY_ORTHO
.elseif CASE == 2
    .byte 0x10
    .long 0x000B0000
    .long 0x3F800000, 0, 0, 0x3E800000
    .long 0, 0x3F800000, 0, 0x3E800000
    .long 0, 0, 0x3F800000, 0
    .byte 0x10
    .long 0x00061020
    .long 0x3F800000, 0, 0x3F800000, 0, 0x3F800000, 0, 1
.elseif CASE == 3
    .byte 0x10
    .long 0x000B0000
    .long 0x3F000000, 0, 0, 0
    .long 0, 0x3F000000, 0, 0
    .long 0, 0, 0x3F800000, 0
    .byte 0x10
    .long 0x00061020
    .long 0x3F800000, 0, 0x3F800000, 0, 0x3F800000, 0, 1
.elseif CASE == 4
    .byte 0x10
    .long 0x000B0000
    .long 0, 0xBF800000, 0, 0
    .long 0x3F800000, 0, 0, 0
    .long 0, 0, 0x3F800000, 0
    .byte 0x10
    .long 0x00061020
    .long 0x3F800000, 0, 0x3F800000, 0, 0x3F800000, 0, 1
.elseif CASE == 5
    XF_IDENTITY_ORTHO
    .byte 0x10
    .long 0x0005101A
    .long 0x43700000, 0xC3700000, 0x4B7FFFFF
    .long 0x43A00000, 0x43700000, 0
.elseif CASE == 6
    .byte 0x10
    .long 0x000B0000
    .long 0x3F800000, 0, 0, 0
    .long 0, 0x3F800000, 0, 0
    .long 0, 0, 0x3F800000, 0xC0000000
    .byte 0x10
    .long 0x00061020
    .long 0x3F800000, 0, 0x3F800000, 0, 0x3F800000, 0, 0
.elseif CASE == 7
    XF_IDENTITY_ORTHO
    CP_LOAD 0x30, 12
    .byte 0x10
    .long 0x000B000C
    .long 0x3F000000, 0, 0, 0
    .long 0, 0x3F000000, 0, 0
    .long 0, 0, 0x3F800000, 0
.endif

.byte 0x90
.short 3
VERTEX_F32 0xBF400000, 0xBF400000, 0, 0xFF3030FF
VERTEX_F32 0x3F400000, 0xBF400000, 0, 0x30FF30FF
VERTEX_F32 0, 0x3F400000, 0, 0x3030FFFF
GX_END

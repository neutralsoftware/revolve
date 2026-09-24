.macro GX_PROGRAM
.section .text
.global _start
_start:
    lis 3, 0xCC00
    li 4, 0
    sth 4, 0x02(3)
    li 4, 1
    sth 4, 0x22(3)
    li 4, 0
    sth 4, 0x20(3)
    li 4, 1
    sth 4, 0x26(3)
    li 4, 0x3FE0
    sth 4, 0x24(3)
    li 4, 1
    sth 4, 0x36(3)
    li 4, gx_stream_end-gx_stream
    sth 4, 0x34(3)
    li 4, 1
    sth 4, 0x3A(3)
    li 4, 0
    sth 4, 0x38(3)
    sth 4, 0x32(3)
    lis 9, 0xCC00
    ori 9, 9, 0x3000
    lis 10, 1
    stw 10, 0x0C(9)
    ori 10, 10, 0x3FE0
    stw 10, 0x10(9)
    lis 10, 1
    addi 10, 10, gx_stream_end-gx_stream
    stw 10, 0x14(9)
    TEST_SETUP
    li 4, gx_stream_end-gx_stream
    sth 4, 0x30(3)
    lis 5, gx_stream@ha
    addi 5, 5, gx_stream@l
    lis 6, 0x8001
    li 7, gx_stream_end-gx_stream
1:
    lbz 8, 0(5)
    stb 8, 0(6)
    addi 5, 5, 1
    addi 6, 6, 1
    addic. 7, 7, -1
    bne 1b
    .ifdef GX_START_CONTROL
    li 4, GX_START_CONTROL
    .else
    li 4, 0x11
    .endif
    sth 4, 0x02(3)
2:
    b 2b
.section .data
.balign 32
gx_stream:
    BP_LOAD 0xC0, 0x08FFFA
    BP_LOAD 0xC1, 0x08FFD0
.endm

.macro GX_END
    .byte 0x61
    .long 0x49000000
    .byte 0x61
    .long 0x4A083E7F
    .byte 0x61
    .long 0x4B000800
    .byte 0x61
    .long 0x4D000028
    .byte 0x61
    .long 0x52004800
    .byte 0x61
    .long 0x4700BEEF
    .balign 32, 0
gx_stream_end:
.endm

.macro CP_LOAD reg, value
    .byte 0x08
    .byte \reg
    .long \value
.endm

.macro BP_LOAD reg, value
    .byte 0x61
    .long ((\reg << 24) | (\value & 0x00FFFFFF))
.endm

.macro XF_IDENTITY_ORTHO
    .byte 0x10
    .long 0x000B0000
    .long 0x3F800000, 0, 0, 0
    .long 0, 0x3F800000, 0, 0
    .long 0, 0, 0x3F800000, 0
    .byte 0x10
    .long 0x00061020
    .long 0x3F800000, 0, 0x3F800000, 0, 0x3F800000, 0
    .long 1
.endm

.macro GX_DIRECT_STATE vat=0x00014009
    CP_LOAD 0x30, 0
    CP_LOAD 0x50, 0x00002200
    CP_LOAD 0x60, 0
    CP_LOAD 0x70, \vat
    XF_IDENTITY_ORTHO
.endm

.macro VERTEX_F32 x, y, z, color
    .long \x, \y, \z, \color
.endm

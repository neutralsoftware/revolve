.include "common.inc"
.include "texture_common.inc"

.macro TEST_SETUP
    TEXTURE_TEST_SETUP
.endm

GX_PROGRAM
TEXTURED_DIRECT_STATE
TEV_ORDER0
TEV_REPLACE

.if CASE == 1
    TEXTURE0 0x0
.elseif CASE == 2
    TEXTURE0 0x1
.elseif CASE == 3
    TEXTURE0 0x2
.elseif CASE == 4
    TEXTURE0 0x3
.elseif CASE == 5
    TEXTURE0 0x4
.elseif CASE == 6
    TEXTURE0 0x5
.elseif CASE == 7
    TEXTURE0 0x6
.elseif CASE == 8
    TEXTURE0 0x8
    TLUT0 1
.elseif CASE == 9
    TEXTURE0 0x9
    TLUT0 2
.elseif CASE == 10
    TEXTURE0 0xA
    TLUT0 0
.elseif CASE == 11
    TEXTURE0 0xE
.endif

.byte 0x80
.short 4
TEXTURED_VERTEX 0xBF400000, 0xBF400000, 0xFFFFFFFF, 0, 0x3F800000
TEXTURED_VERTEX 0x3F400000, 0xBF400000, 0xFFFFFFFF, 0x3F800000, 0x3F800000
TEXTURED_VERTEX 0x3F400000, 0x3F400000, 0xFFFFFFFF, 0x3F800000, 0
TEXTURED_VERTEX 0xBF400000, 0x3F400000, 0xFFFFFFFF, 0, 0
GX_END

.balign 32
texture_data:
.if CASE == 1
    .rept 4
        .byte 0xF0, 0xF0, 0xF0, 0xF0
        .byte 0x0F, 0x0F, 0x0F, 0x0F
    .endr
.elseif CASE == 2
    .rept 4
        .byte 0x00, 0x24, 0x48, 0x6C, 0x90, 0xB4, 0xD8, 0xFF
    .endr
    .rept 4
        .byte 0xFF, 0xD8, 0xB4, 0x90, 0x6C, 0x48, 0x24, 0x00
    .endr
.elseif CASE == 3
    .rept 4
        .byte 0xFF, 0xF8, 0xF4, 0xF0, 0x8F, 0x88, 0x84, 0x80
    .endr
    .rept 4
        .byte 0x4F, 0x48, 0x44, 0x40, 0x0F, 0x08, 0x04, 0x00
    .endr
.elseif CASE == 4
    .rept 16
        .short 0xFFFF
    .endr
    .rept 16
        .short 0x80FF
    .endr
    .rept 16
        .short 0xFF80
    .endr
    .rept 16
        .short 0x4080
    .endr
.elseif CASE == 5
    RGB565_BLOCK 0xF800
    RGB565_BLOCK 0x07E0
    RGB565_BLOCK 0x001F
    RGB565_BLOCK 0xFFFF
.elseif CASE == 6
    RGB5A3_BLOCK 0xFC00
    RGB5A3_BLOCK 0x83E0
    RGB5A3_BLOCK 0x801F
    RGB5A3_BLOCK 0x0FFF
.elseif CASE == 7
    RGBA8_TEST_TEXTURE
.elseif CASE == 8
    .rept 4
        .byte 0x01, 0x23, 0x01, 0x23
        .byte 0x32, 0x10, 0x32, 0x10
    .endr
.elseif CASE == 9
    .rept 4
        .byte 0, 1, 2, 3, 0, 1, 2, 3
    .endr
    .rept 4
        .byte 3, 2, 1, 0, 3, 2, 1, 0
    .endr
.elseif CASE == 10
    .rept 16
        .short 0, 1, 2, 3
    .endr
.elseif CASE == 11
    .short 0xF800, 0x001F
    .long 0x00000000
    .short 0x07E0, 0x001F
    .long 0x55555555
    .short 0x001F, 0xFFFF
    .long 0xAAAAAAAA
    .short 0xFFFF, 0x0000
    .long 0xFFFFFFFF
.endif
texture_data_end:

EMPTY_SECOND_TEXTURE
TEST_PALETTE
texcoord_data:
    .long 0, 0
texcoord_data_end:

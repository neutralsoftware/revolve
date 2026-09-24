.include "common.inc"
.include "texture_common.inc"

.macro ADVANCED_SETUP
    TEXTURE_TEST_SETUP
.endm

.macro ADVANCED_STATE texgens=1, tevstages=1, channels=0
    CP_LOAD 0x30, 0
    CP_LOAD 0x50, 0x00002200
    CP_LOAD 0x60, 0x00000005
    CP_LOAD 0x70, 0x01214009
    CP_LOAD 0x80, 0x00000009
    XF_IDENTITY_ORTHO
    .byte 0x10
    .long 0x00080400
    .long 0x3F800000, 0, 0
    .long 0, 0x3F800000, 0
    .long 0, 0, 0x3F800000
    .byte 0x10
    .long 0x0000103F
    .long \texgens
    .byte 0x10
    .long 0x00011040
    .long 0x00000280, 0x00000300
    BP_LOAD 0x00, ((((\tevstages) - 1) << 10) | ((\channels) << 4) | (\texgens))
.endm

.macro ADVANCED_TEXTURES
    TEXTURE0 0x6, 0x5
    TEXTURE1 0x6, 0x5
.endm

.macro TEV_ORDER01 map0=0, coord0=0, channel0=0, map1=1, coord1=1, channel1=0
    BP_LOAD 0x28, ((\map0) | ((\coord0) << 3) | 0x40 | ((\channel0) << 7) | ((\map1) << 12) | ((\coord1) << 15) | 0x40000 | ((\channel1) << 19))
.endm

.macro TEV_ORDER23 map2=0, coord2=0, channel2=0, map3=1, coord3=1, channel3=0
    BP_LOAD 0x29, ((\map2) | ((\coord2) << 3) | 0x40 | ((\channel2) << 7) | ((\map3) << 12) | ((\coord3) << 15) | 0x40000 | ((\channel3) << 19))
.endm

.macro POST_MATRIX_IDENTITY
    .byte 0x10
    .long 0x000B0500
    .long 0x3F800000, 0, 0, 0
    .long 0, 0x3F800000, 0, 0
    .long 0, 0, 0x3F800000, 0
.endm

.macro DRAW_ADVANCED_QUAD z=0
    .byte 0x80
    .short 4
    .long 0xBF400000, 0xBF400000, \z, 0xFF6060D0, 0, 0x3F800000, 0x40000000, 0
    .long 0x3F400000, 0xBF400000, \z, 0x60FF60D0, 0x3F800000, 0x3F800000, 0, 0x40000000
    .long 0x3F400000, 0x3F400000, \z, 0x6060FFD0, 0x3F800000, 0, 0xC0000000, 0
    .long 0xBF400000, 0x3F400000, \z, 0xFFFFFFFF, 0, 0, 0, 0xC0000000
.endm

.macro DRAW_ADVANCED_SOLID z, color
    .byte 0x80
    .short 4
    .long 0xBF400000, 0xBF400000, \z, \color, 0, 0x3F800000, 0x40000000, 0
    .long 0x3F400000, 0xBF400000, \z, \color, 0x3F800000, 0x3F800000, 0, 0x40000000
    .long 0x3F400000, 0x3F400000, \z, \color, 0x3F800000, 0, 0xC0000000, 0
    .long 0xBF400000, 0x3F400000, \z, \color, 0, 0, 0, 0xC0000000
.endm

.macro ADVANCED_DATA
.balign 32
texture_data:
    RGBA8_TEST_TEXTURE
texture_data_end:
texture_data_1:
    RGBA8_BLOCK 0xFF, 0xFF, 0x20, 0xFF
    RGBA8_BLOCK 0x20, 0xFF, 0xFF, 0xC0
    RGBA8_BLOCK 0xFF, 0x20, 0xFF, 0x80
    RGBA8_BLOCK 0x20, 0x20, 0x20, 0x40
texture_data_1_end:
TEST_PALETTE
texcoord_data:
    .long 0, 0
texcoord_data_end:
.endm

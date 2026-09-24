.include "common.inc"
.include "texture_common.inc"

.macro ADVANCED_SETUP
    TEXTURE_TEST_SETUP
.endm

.macro ADVANCED_STATE texgens=1, tevstages=1
    CP_LOAD 0x30, 0
    CP_LOAD 0x50, 0x00002200
    CP_LOAD 0x60, 0x00000005
    CP_LOAD 0x70, 0x01214009
    CP_LOAD 0x80, 0x00000009
    XF_IDENTITY_ORTHO
    .byte 0x10
    .long 0x0000103F
    .long \texgens
    .byte 0x10
    .long 0x00011040
    .long 0x00000280, 0x00000300
    BP_LOAD 0x00, ((((\tevstages) - 1) << 10) | 1)
.endm

.macro ADVANCED_TEXTURES
    TEXTURE0 0x6, 0x5
    TEXTURE1 0x6, 0x5
.endm

.macro DRAW_ADVANCED_QUAD z=0
    .byte 0x80
    .short 4
    .long 0xBF400000, 0xBF400000, \z, 0xFF6060D0, 0, 0x3F800000, 0x40000000, 0
    .long 0x3F400000, 0xBF400000, \z, 0x60FF60D0, 0x3F800000, 0x3F800000, 0, 0x40000000
    .long 0x3F400000, 0x3F400000, \z, 0x6060FFD0, 0x3F800000, 0, 0xC0000000, 0
    .long 0xBF400000, 0x3F400000, \z, 0xFFFFFFFF, 0, 0, 0, 0xC0000000
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

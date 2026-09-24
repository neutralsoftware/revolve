.macro COPY_GUEST_BLOCK start, end, destination
    lis 5, \start@ha
    addi 5, 5, \start@l
    lis 6, \destination
    li 7, \end-\start
991:
    lbz 8, 0(5)
    stb 8, 0(6)
    addi 5, 5, 1
    addi 6, 6, 1
    addic. 7, 7, -1
    bne 991b
.endm

.macro TEXTURE_TEST_SETUP
    COPY_GUEST_BLOCK texture_data, texture_data_end, 0x8002
    COPY_GUEST_BLOCK texture_data_1, texture_data_1_end, 0x8004
    COPY_GUEST_BLOCK palette_data, palette_data_end, 0x8003
    COPY_GUEST_BLOCK texcoord_data, texcoord_data_end, 0x8005
.endm

.macro TEXTURED_DIRECT_STATE vat=0x01214009
    CP_LOAD 0x30, 0
    CP_LOAD 0x50, 0x00002200
    CP_LOAD 0x60, 0x00000001
    CP_LOAD 0x70, \vat
    XF_IDENTITY_ORTHO
    .byte 0x10
    .long 0x0000103F
    .long 1
    .byte 0x10
    .long 0x00001040
    .long 0x00000280
    BP_LOAD 0x00, 0x000001
.endm

.macro TEXTURE0 format, mode=0, width=8, height=8
    BP_LOAD 0x80, \mode
    BP_LOAD 0x84, 0
    BP_LOAD 0x88, (((\format) << 20) | (((\height) - 1) << 10) | ((\width) - 1))
    BP_LOAD 0x8C, 0x0D8000
    BP_LOAD 0x90, 0x0DC000
    BP_LOAD 0x94, 0x001000
.endm

.macro TEXTURE1 format, mode=0, width=8, height=8
    BP_LOAD 0x81, \mode
    BP_LOAD 0x85, 0
    BP_LOAD 0x89, (((\format) << 20) | (((\height) - 1) << 10) | ((\width) - 1))
    BP_LOAD 0x8D, 0x0D8400
    BP_LOAD 0x91, 0x0DC400
    BP_LOAD 0x95, 0x002000
.endm

.macro TLUT0 format
    BP_LOAD 0x64, 0x001800
    BP_LOAD 0x65, 0x000600
    BP_LOAD 0x98, (((\format) << 10) | 0x200)
.endm

.macro TEV_ORDER0 map=0, coord=0, channel=0
    BP_LOAD 0x28, ((\map) | ((\coord) << 3) | 0x40 | ((\channel) << 7))
.endm

.macro TEV_ORDER1 map=0, coord=0, channel=0
    BP_LOAD 0x28, (((\map) << 12) | ((\coord) << 15) | 0x40000 | ((\channel) << 19))
.endm

.macro TEV_REPLACE stage=0
    BP_LOAD (0xC0 + ((\stage) * 2)), 0x08FFF8
    BP_LOAD (0xC1 + ((\stage) * 2)), 0x08FFC0
.endm

.macro TEV_MODULATE stage=0
    BP_LOAD (0xC0 + ((\stage) * 2)), 0x08F8AF
    BP_LOAD (0xC1 + ((\stage) * 2)), 0x08F2F0
.endm

.macro TEXTURED_VERTEX x, y, color, u, v
    .long \x, \y, 0, \color, \u, \v
.endm

.macro RGBA8_BLOCK r, g, b, a
    .rept 16
        .byte \a, \r
    .endr
    .rept 16
        .byte \g, \b
    .endr
.endm

.macro RGBA8_TEST_TEXTURE
    RGBA8_BLOCK 0xFF, 0x20, 0x20, 0xFF
    RGBA8_BLOCK 0x20, 0xFF, 0x20, 0xC0
    RGBA8_BLOCK 0x20, 0x20, 0xFF, 0x80
    RGBA8_BLOCK 0xFF, 0xFF, 0xFF, 0x40
.endm

.macro RGB565_BLOCK color
    .rept 16
        .short \color
    .endr
.endm

.macro RGB5A3_BLOCK color
    .rept 16
        .short \color
    .endr
.endm

.macro EMPTY_SECOND_TEXTURE
texture_data_1:
    RGBA8_TEST_TEXTURE
texture_data_1_end:
.endm

.macro TEST_PALETTE
palette_data:
    .short 0xF800, 0x07E0, 0x001F, 0xFFFF
    .short 0xFC00, 0x83E0, 0x801F, 0x0000
    .rept 248
        .short 0xFFFF
    .endr
palette_data_end:
.endm

.include "common.inc"

.if CASE == 13
.set GX_START_CONTROL, 0x1D
.endif

.macro TEST_SETUP
    .if CASE == 9
        lis 11, 0xCC00
        ori 11, 11, 0x8000
        li 12, 0
        .rept 8
            stw 12, 0(11)
        .endr
    .elseif CASE == 10
        lis 11, 0xCC00
        ori 11, 11, 0x8000
        lis 12, 0x0800
        stw 12, 0(11)
        li 12, 0
        .rept 6
            stw 12, 0(11)
        .endr
        sth 12, 0(11)
        stb 12, 0(11)
        stb 12, 0(11)
    .elseif CASE == 11
        lis 11, 0xCC00
        ori 11, 11, 0x8000
        li 12, 0
        .rept 16
            stw 12, 0(11)
        .endr
    .elseif CASE == 12
        li 12, 0x0100
        sth 12, 0x2A(3)
        li 12, 0
        sth 12, 0x28(3)
        li 12, 0
        sth 12, 0x2E(3)
        li 12, 0x0040
        sth 12, 0x2C(3)
    .elseif CASE == 13
        li 12, 0x000C
        sth 12, 0x02(3)
        li 12, 3
        sth 12, 0x04(3)
    .endif
.endm

GX_PROGRAM

.if CASE == 1
    .rept 32
    .byte 0
    .endr
.elseif CASE == 2
    CP_LOAD 0x50, 0x00002200
    CP_LOAD 0x60, 0
    CP_LOAD 0x70, 0x00014009
.elseif CASE == 3
    BP_LOAD 0x40, 0x123456
    BP_LOAD 0x41, 0x654321
.elseif CASE == 4
    .byte 0x10
    .long 0x000B0000
    .long 0x3F800000, 0, 0, 0
    .long 0, 0x3F800000, 0, 0
    .long 0, 0, 0x3F800000, 0
.elseif CASE == 5
    .byte 0x10
    .long 0x00061020
    .long 0x3F800000, 0, 0x3F800000, 0, 0x3F800000, 0, 1
.elseif CASE == 6
    GX_DIRECT_STATE
    .rept 64
    .byte 0
    .endr
.elseif CASE == 7
    GX_DIRECT_STATE
    BP_LOAD 0x45, 0x000002
    BP_LOAD 0x46, 0x000010
    BP_LOAD 0x47, 0x00FFFFFF
.elseif CASE == 8
    GX_DIRECT_STATE
    CP_LOAD 0xA0, 0x00012000
    CP_LOAD 0xB0, 12
.elseif CASE == 9
    .rept 32
    .byte 0
    .endr
.elseif CASE == 10
    .rept 32
    .byte 0
    .endr
.elseif CASE == 11
    .rept 64
    .byte 0
    .endr
.elseif CASE == 12
    .rept 32
    .byte 0
    .endr
.elseif CASE == 13
    .rept 32
    .byte 0
    .endr
.endif

GX_END

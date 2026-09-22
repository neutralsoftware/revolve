.include "common.inc"

.macro TEST_SETUP
    lis 9, 0xCC00
    ori 9, 9, 0x2000
    .if CASE == 1
        li 10, 0x0F06
        sth 10, 0x00(9)
    .elseif CASE == 2
        li 10, 1
        sth 10, 0x02(9)
    .elseif CASE == 3
        lis 10, 0x4769
        ori 10, 10, 0x01AD
        stw 10, 0x04(9)
    .elseif CASE == 4
        lis 10, 0x02EA
        ori 10, 10, 0x5140
        stw 10, 0x08(9)
    .elseif CASE == 5
        lis 10, 0x0001
        ori 10, 10, 0x0000
        stw 10, 0x1C(9)
    .elseif CASE == 6
        lis 10, 0x0001
        ori 10, 10, 0x2800
        stw 10, 0x24(9)
    .elseif CASE == 7
        lhz 10, 0x2C(9)
        sth 10, 0x00(9)
    .elseif CASE == 8
        lhz 10, 0x2E(9)
        sth 10, 0x02(9)
    .elseif CASE == 9
        lis 11, 0xCC00
        ori 11, 11, 0x3004
        li 12, 0x0100
        stw 12, 0(11)
        lis 10, 0x1002
        ori 10, 10, 0x0001
        stw 10, 0x30(9)
    .elseif CASE == 10
        li 10, 0
        stw 10, 0x30(9)
    .elseif CASE == 11
        lis 11, 0xCC00
        ori 11, 11, 0x3004
        li 12, 0x0100
        stw 12, 0(11)
        lis 10, 0x1002
        ori 10, 10, 0x0001
        stw 10, 0x30(9)
        lis 10, 0x1004
        ori 10, 10, 0x0001
        stw 10, 0x34(9)
        lis 10, 0x1006
        ori 10, 10, 0x0001
        stw 10, 0x38(9)
        lis 10, 0x1008
        ori 10, 10, 0x0001
        stw 10, 0x3C(9)
    .elseif CASE == 12
        li 10, 0
        stw 10, 0x30(9)
        stw 10, 0x34(9)
        stw 10, 0x38(9)
        stw 10, 0x3C(9)
    .endif
.endm

GX_PROGRAM

GX_DIRECT_STATE
.byte 0x90
.short 3
VERTEX_F32 0xBF400000, 0xBF400000, 0, 0xFF0000FF
VERTEX_F32 0x3F400000, 0xBF400000, 0, 0x00FF00FF
VERTEX_F32 0, 0x3F400000, 0, 0x0000FFFF
GX_END

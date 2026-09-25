.section .text
.global _start
_start:
.if CASE == 1
    lis 3, 0xCD00
    ori 3, 3, 0x6400
    lis 4, 0x8000
    ori 4, 4, 0x3000
    lis 5, 0x4354
    ori 5, 5, 0x524C
1:
    lis 6, 0x4003
    stw 6, 0x80(3)
    lis 6, 0x0803
    ori 6, 6, 1
    stw 6, 0x34(3)
    lwz 7, 0x80(3)
    lwz 8, 0x84(3)
    stw 5, 0(4)
    stw 7, 4(4)
    stw 8, 8(4)
    b 1b
.else
    lis 3, 0xCD80
    lis 4, open_request@ha
    addi 4, 4, open_request@l
    stw 4, 0(3)
    li 5, 1
    stw 5, 4(3)
1:
    lwz 5, 0(4)
    cmpwi 5, 8
    bne 1b
    lwz 6, 4(4)
    lis 4, read_request@ha
    addi 4, 4, read_request@l
    stw 6, 8(4)
2:
    li 5, 3
    stw 5, 0(4)
    stw 6, 8(4)
    stw 4, 0(3)
    li 5, 1
    stw 5, 4(3)
3:
    lwz 5, 0(4)
    cmpwi 5, 8
    bne 3b
    li 5, 4
    stw 5, 4(3)
    b 2b
.endif

.section .data
.balign 32
open_request:
    .long 1, 0, -1, wiimote_path, 0, 0, 0, 0
read_request:
    .long 3, 0, 0, 0x80003000, 24, 0, 0, 0
wiimote_path:
    .asciz "/dev/revolve/wiimote"

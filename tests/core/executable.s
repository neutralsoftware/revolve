.section .text
.global _start

_start:
    li 3, 10
    li 4, 20
    add 5, 3, 4

loop:
    b loop

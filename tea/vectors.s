
.globl _start
_start:
    rjmp reset


reset:
    rcall notmain
    .word 0xFFFF
1:
    rjmp 1b

.globl dummy
dummy:
    ldi r24,0x00
1:
    inc r24
    brne 1b

    ret


.globl subtest
subtest:

    ldi r24,0x00
    ldi r25,0x00
    sts 0x5F,r24
    dec r25
    lds r24,0x5F
    ret





.globl _start
_start:
    rjmp reset


reset:
    rcall notmain
1:
    rjmp 1b

.globl dummy
dummy:
    ldi r24,0x00
1:
    inc r24
    brne 1b

    ret


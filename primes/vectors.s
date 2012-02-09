
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

; ldi r23,0x00
;    sts 0x5F,r24
    add r22,r24
    lds r24,0x5F
    ret

















        push    r28
        push    r29
        ldi r28, 0x00   ; 0
        ldi r29, 0x01   ; 1
        mov r26, r28
        mov r27, r29
lab8c:
        st  X+, r1
        ldi r24, 0x01   ; 1
        cpi r26, 0x64   ; 100
        cpc r27, r24
        brne lab8c
        ldi r24, 0x01   ; 1
        sts 0x0100, r24
        ldi r30, 0x01   ; 1
        ldi r31, 0x01   ; 1
        ldi r25, 0x00   ; 0
        cp  r26, r30
        cpc r27, r31
        breq   labb8
laba8:
        ld  r24, Z
        and r24, r24
        brne    labb0
        subi    r25, 0xFF   ; 255
labb0:
        adiw    r30, 0x01   ; 1
        cp  r26, r30
        cpc r27, r31
        brne    laba8
labb8:
    .word 0xFFFF
        mov r24, r25
  pop r29
  pop r28
  ret





.globl subtestx
subtestx:
    push    r28
    push    r29
    ldi r28, 0x00   ; 0
    ldi r29, 0x01   ; 1
    mov r30, r28
    mov r31, r29
l8c:
    st  Z+, r1
    ldi r24, 0x01   ; 1
    cpi r30, 0x64   ; 100
    cpc r31, r24
    brne    l8c
    ldi r24, 0x01   ; 1
    sts 0x0100, r24
    ldi r30, 0x01   ; 1
    ldi r31, 0x01   ; 1
    ldi r25, 0x00   ; 0
    ldi r24, 0x01   ; 1
    cpi r30, 0x64   ; 100
    cpc r31, r24
    breq    lbc
laa:
    ld  r24, Z
    and r24, r24
    brne    lb2
    subi    r25, 0xFF   ; 255
lb2:
    adiw    r30, 0x01   ; 1
    ldi r24, 0x01   ; 1
    cpi r30, 0x64   ; 100
    cpc r31, r24
    brne    laa
lbc:
    mov r24, r25
    pop r29
    pop r28
    ret











    push    r28
    push    r29
    ldi r28, 0x00   ; 0
    ldi r29, 0x01   ; 1
    mov r26, r28
    mov r27, r29
    st x+,r1
    ldi r24, 0x01
    cpi r26, 0x64
    cpc r27, r24
    brne    subeq


    ldi r24,0x00
    pop r29
    pop r28
    ret
subeq:

    ldi r24,0x01
    pop r29
    pop r28
    ret


    sec
    ldi r24,0x04
    ldi r25,0x04
    cp r24,r25
    lds r24,0x5F
    ret




    sts 0x5F,r24
    dec r25
    lds r24,0x5F
    ret




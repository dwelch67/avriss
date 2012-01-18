
.device ATmega168
.equ  DDRB       = 0x04
.equ  PORTB      = 0x05

.equ    UDR0    = 0xc6  ; MEMORY MAPPED
.equ    UBRR0H  = 0xc5  ; MEMORY MAPPED
.equ    UBRR0L  = 0xc4  ; MEMORY MAPPED
.equ    UCSR0C  = 0xc2  ; MEMORY MAPPED
.equ    UCSR0B  = 0xc1  ; MEMORY MAPPED
.equ    UCSR0A  = 0xc0  ; MEMORY MAPPED

.equ    SREG    = 0x3f
.equ   SHOW = 0x2F

.org 0x0000
    rjmp RESET

RESET:
    ldi R16,0x20
    out DDRB,R16

    ldi R18,0x00
    ldi R17,0x00
    ldi R20,0x20
Loop:

    ldi R19,0xE8
aloop:
 ;   nop
    inc R17
    cpi R17,0x00
    brne aloop

;    inc R18
;    cpi R18,0x00
;    brne aloop
;

;    eor R16,R20
;    out PORTB, R16


;    inc R19
;    cpi R19,0x00
;    brne aloop

    call xjump

    adiw r24,1

xjump:

    ldi r16,0x12
    call doshow

    break;

doshow:
    sts SHOW,r16
    ret


    break



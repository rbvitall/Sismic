;-------------------------------------------------------------------------------
; Módulo 1 - Exemplo 2: produto de dois bytes sem sinal por somas sucessivas
; R4 = X, R5 = Y (1 byte cada) -> R6 = X * Y (soma Y vezes o valor de X)
;-------------------------------------------------------------------------------
        .cdecls "msp430.h"
        .global main
        .text

main:
        mov.w #(WDTPW|WDTHOLD), &WDTCTL

        mov.b #25, R4       ; X (byte sem sinal)
        mov.b #11, R5       ; Y (byte sem sinal)

        clr.w R6            ; Produto = 0
        tst.w R5            ; Se Y = 0, o produto já é 0
        jz    FIM
LOOP:
        add.w R4, R6        ; Produto += X
        dec.w R5            ; Y = Y - 1
        jnz   LOOP          ; Enquanto Y != 0

FIM:
        jmp   $             ; Loop infinito (R6 = 275)
        nop

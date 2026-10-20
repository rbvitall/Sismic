;-------------------------------------------------------------------------------
; Módulo 1 - Exercício 4: multiplicação por somas sucessivas
; R6 = R4 * R5, com R4 e R5 limitados a 1 byte (resultado cabe em 16 bits)
;-------------------------------------------------------------------------------
            .cdecls "msp430.h"
            .global main
            .text
main:
            mov.w   #(WDTPW|WDTHOLD), &WDTCTL

            mov.b   #200, R4        ; X (8 bits, sem sinal)
            mov.b   #150, R5        ; Y (8 bits, sem sinal)

            clr.w   R6              ; Acumulador do produto
            tst.w   R5              ; Y = 0 -> produto = 0
            jz      FIM
LOOP:
            add.w   R4, R6          ; R6 += X
            dec.w   R5              ; Y--
            jnz     LOOP            ; Repete enquanto Y != 0
FIM:
            jmp     $               ; R6 = 30000
            nop

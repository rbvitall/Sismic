;-------------------------------------------------------------------------------
; Módulo 1 - Exercício 6: sub-rotina mult8
; Entrada: R12 = operando A (8 bits), R13 = operando B (8 bits)
; Saída:   R12 = A * B (16 bits)
;-------------------------------------------------------------------------------
            .cdecls "msp430.h"
            .global main
            .text
main:
            mov.w   #(WDTPW|WDTHOLD), &WDTCTL

            mov.w   #0, R12         ; Caso extremo: 0 * 37 = 0
            mov.w   #37, R13
            call    #mult8
            mov.w   R12, R4

            mov.w   #255, R12       ; Caso extremo: 255 * 255 = 65025
            mov.w   #255, R13
            call    #mult8
            mov.w   R12, R5

            mov.w   #12, R12        ; Caso intermediário: 12 * 34 = 408
            mov.w   #34, R13
            call    #mult8
            mov.w   R12, R6

            jmp     $
            nop

;-------------------------------------------------------------------------------
; mult8: R12 = R12 * R13 (somas sucessivas)
;-------------------------------------------------------------------------------
mult8:
            push.w  R4
            clr.w   R4              ; Acumulador
            and.w   #0x00FF, R12    ; Garante operandos de 8 bits
            and.w   #0x00FF, R13
            jz      mult8_fim       ; B = 0 -> resultado 0
mult8_loop:
            add.w   R12, R4
            dec.w   R13
            jnz     mult8_loop
mult8_fim:
            mov.w   R4, R12
            pop.w   R4
            ret

;-------------------------------------------------------------------------------
; Módulo 1 - Exercício 10: sub-rotina reduceSum8
; Entrada: R12 = endereço do vetor de bytes, R13 = número de elementos
; Saída:   R12 = soma de todos os bytes (16 bits)
;-------------------------------------------------------------------------------
            .cdecls "msp430.h"
            .global main
            .text
main:
            mov.w   #(WDTPW|WDTHOLD), &WDTCTL

            mov.w   #vetor, R12
            mov.w   #10, R13
            call    #reduceSum8
            jmp     $               ; 1+2+...+10 = 55
            nop

;-------------------------------------------------------------------------------
reduceSum8:
            push.w  R4
            push.w  R5
            clr.w   R4              ; Acumulador de 16 bits
            tst.w   R13
            jz      rs8_fim
rs8_loop:
            mov.b   @R12+, R5       ; Lê o byte (zera a parte alta de R5)
            add.w   R5, R4          ; Soma em 16 bits
            dec.w   R13
            jnz     rs8_loop        ; Enquanto houver elementos
rs8_fim:
            mov.w   R4, R12
            pop.w   R5
            pop.w   R4
            ret

;-------------------------------------------------------------------------------
            .data
vetor:      .byte   1, 2, 3, 4, 5, 6, 7, 8, 9, 10

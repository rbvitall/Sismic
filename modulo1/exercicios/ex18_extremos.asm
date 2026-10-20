;-------------------------------------------------------------------------------
; Módulo 1 - Exercício 18: sub-rotina extremos
; Entrada: R12 = endereço de um vetor de words COM sinal, R13 = tamanho
; Saída:   R12 = menor elemento, R13 = maior elemento
; As comparações com sinal usam JGE/JL (testam N xor V).
;-------------------------------------------------------------------------------
            .cdecls "msp430.h"
            .global main
            .text
main:
            mov.w   #(WDTPW|WDTHOLD), &WDTCTL

            mov.w   #vetor, R12
            mov.w   #8, R13
            call    #extremos
            jmp     $               ; R12 = -32768, R13 = 30000
            nop

;-------------------------------------------------------------------------------
extremos:
            push.w  R4
            push.w  R5
            push.w  R6
            mov.w   @R12, R4        ; Menor
            mov.w   R4, R5          ; Maior
ext_loop:
            mov.w   @R12+, R6
            cmp.w   R4, R6          ; R6 - menor
            jge     ext_naoMenor
            mov.w   R6, R4
ext_naoMenor:
            cmp.w   R5, R6          ; R6 - maior
            jl      ext_naoMaior
            mov.w   R6, R5
ext_naoMaior:
            dec.w   R13
            jnz     ext_loop
            mov.w   R4, R12
            mov.w   R5, R13
            pop.w   R6
            pop.w   R5
            pop.w   R4
            ret

;-------------------------------------------------------------------------------
            .data
vetor:      .word   15, -200, 30000, 0, -32768, 7, -1, 29999

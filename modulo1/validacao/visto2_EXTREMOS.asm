;-------------------------------------------------------------------------------
; Módulo 1 - Visto 2: sub-rotina EXTREMOS
; Entrada: R12 = endereço de um vetor de words COM sinal, R13 = quantidade
; Saída:   R12 = menor elemento, R13 = maior elemento
; Comparações com sinal: JGE / JL (usam N xor V em vez do carry).
;-------------------------------------------------------------------------------
        .cdecls "msp430.h"
        .global main
        .text

main:
        mov.w   #(WDTPW|WDTHOLD), &WDTCTL

        mov.w   #vetor, R12
        mov.w   #8, R13
        call    #EXTREMOS
        jmp     $               ; R12 = -32768 (0x8000), R13 = 30000 (0x7530)
        nop

;-------------------------------------------------------------------------------
; R4 = menor, R5 = maior, R6 = elemento atual
;-------------------------------------------------------------------------------
EXTREMOS:
        push.w  R4
        push.w  R5
        push.w  R6
        mov.w   @R12, R4        ; Menor e maior começam no primeiro elemento
        mov.w   R4, R5
ext_loop:
        mov.w   @R12+, R6
        cmp.w   R4, R6          ; R6 - menor
        jge     ext_naoMenor    ; R6 >= menor
        mov.w   R6, R4          ; Novo menor
ext_naoMenor:
        cmp.w   R5, R6          ; R6 - maior
        jl      ext_naoMaior    ; R6 < maior
        mov.w   R6, R5          ; Novo maior
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
vetor:  .word   15, -200, 30000, 0, -32768, 7, -1, 29999

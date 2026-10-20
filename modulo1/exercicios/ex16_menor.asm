;-------------------------------------------------------------------------------
; Módulo 1 - Exercício 16: sub-rotina menor
; Entrada: R12 = endereço de um vetor de bytes sem sinal, R13 = tamanho
; Saída:   R12 = menor elemento, R13 = frequência (quantas vezes apareceu)
;-------------------------------------------------------------------------------
            .cdecls "msp430.h"
            .global main
            .text
main:
            mov.w   #(WDTPW|WDTHOLD), &WDTCTL

            mov.w   #vetor, R12
            mov.w   #10, R13
            call    #menor
            jmp     $               ; R12 = 3, R13 = 3
            nop

;-------------------------------------------------------------------------------
menor:
            push.w  R4
            push.w  R5
            push.w  R6
            mov.b   @R12, R4        ; Menor = primeiro elemento
            clr.w   R5              ; Frequência (o 1º elemento é contado no laço)
menor_loop:
            mov.b   @R12+, R6
            cmp.b   R4, R6          ; R6 - R4
            jlo     menor_novo      ; R6 < menor (sem sinal)
            jne     menor_prox      ; R6 > menor
            inc.w   R5              ; R6 == menor
            jmp     menor_prox
menor_novo:
            mov.w   R6, R4
            mov.w   #1, R5
menor_prox:
            dec.w   R13
            jnz     menor_loop
            mov.w   R4, R12
            mov.w   R5, R13
            pop.w   R6
            pop.w   R5
            pop.w   R4
            ret

;-------------------------------------------------------------------------------
            .data
vetor:      .byte   200, 3, 45, 3, 255, 128, 3, 90, 17, 250

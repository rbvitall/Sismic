;-------------------------------------------------------------------------------
; Módulo 1 - Exercício 21: sub-rotina ORDENA (método da bolha)
; Entrada: R12 = endereço de um vetor de bytes sem sinal, R13 = tamanho
; Ordena o vetor em ordem crescente, no próprio lugar.
; São N-1 varridas; a varrida k faz N-k comparações, pois o maior elemento
; de cada varrida já fica no final do vetor.
;-------------------------------------------------------------------------------
            .cdecls "msp430.h"
            .global main
            .text
main:
            mov.w   #(WDTPW|WDTHOLD), &WDTCTL

            mov.w   #vetor, R12
            mov.w   #10, R13
            call    #ORDENA
            jmp     $               ; 0, 1, 3, 4, 5, 7, 99, 128, 200, 255
            nop

;-------------------------------------------------------------------------------
; R4 = comparações da varrida atual, R5 = ponteiro, R6 = contador, R7 = temp
;-------------------------------------------------------------------------------
ORDENA:
            push.w  R4
            push.w  R5
            push.w  R6
            push.w  R7
            cmp.w   #2, R13
            jlo     ord_fim         ; 0 ou 1 elemento: já ordenado
            mov.w   R13, R4
            dec.w   R4              ; Primeira varrida: N-1 comparações
ord_varrida:
            mov.w   R12, R5
            mov.w   R4, R6
ord_compara:
            mov.b   0(R5), R7       ; R7 = v[i]
            cmp.b   1(R5), R7       ; v[i] - v[i+1]
            jlo     ord_prox        ; v[i] < v[i+1]: ok
            jeq     ord_prox        ; v[i] == v[i+1]: ok
            mov.b   1(R5), 0(R5)    ; Troca v[i] <-> v[i+1]
            mov.b   R7, 1(R5)
ord_prox:
            inc.w   R5
            dec.w   R6
            jnz     ord_compara
            dec.w   R4              ; Próxima varrida é uma comparação menor
            jnz     ord_varrida
ord_fim:
            pop.w   R7
            pop.w   R6
            pop.w   R5
            pop.w   R4
            ret

;-------------------------------------------------------------------------------
            .data
vetor:      .byte   4, 7, 3, 5, 1, 255, 0, 128, 99, 200

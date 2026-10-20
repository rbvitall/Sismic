;-------------------------------------------------------------------------------
; Módulo 1 - Visto 1: sub-rotina reduceSum8
; Retorna o somatório de todos os bytes de um vetor (o resultado cabe em 8 bits)
; Entrada: R12 = endereço do vetor de bytes, R13 = quantidade de elementos
; Saída:   R12 = soma dos elementos
; Segue o fluxograma da Figura 3: R4 acumula a soma (salvo com PUSH/POP),
; R12 é o ponteiro (incrementado a cada iteração) e R13 conta o que falta.
;-------------------------------------------------------------------------------
        .cdecls "msp430.h"
        .global main
        .text

main:
        mov.w   #(WDTPW|WDTHOLD), &WDTCTL

        mov.w   #vetor, R12     ; Endereço do vetor
        mov.w   #10, R13        ; Quantidade de elementos
        call    #reduceSum8
        jmp     $               ; R12 = 55
        nop

;-------------------------------------------------------------------------------
reduceSum8:
        push.w  R4              ; Preserva o valor anterior de R4
        clr.w   R4              ; Soma = 0
        tst.w   R13             ; Vetor vazio?
        jz      rs8_fim
rs8_loop:
        add.b   @R12+, R4       ; Soma += elemento; avança o ponteiro
        dec.w   R13             ; Falta um elemento a menos
        jnz     rs8_loop
rs8_fim:
        mov.w   R4, R12         ; Retorna a soma em R12
        pop.w   R4              ; Restaura R4
        ret

;-------------------------------------------------------------------------------
        .data
vetor:  .byte   1, 2, 3, 4, 5, 6, 7, 8, 9, 10

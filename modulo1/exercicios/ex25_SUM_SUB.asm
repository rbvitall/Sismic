;-------------------------------------------------------------------------------
; Módulo 1 - Exercício 25: sub-rotina SUM_SUB
; Entrada: R12 = ponteiro para a matriz de words, R13 = linhas, R14 = colunas
; Saída (somas excluindo a(s) linha/coluna indicadas):
;   R12: sem 1ª linha e sem 1ª coluna      R13: sem 1ª linha e sem última coluna
;   R14: sem última linha e sem 1ª coluna  R15: sem última linha e sem última coluna
; Ex.: [1 2 3; 4 5 6; 7 8 9] -> R12 = 28, R13 = 24, R14 = 16, R15 = 12
;-------------------------------------------------------------------------------
            .cdecls "msp430.h"
            .global main
            .text
main:
            mov.w   #(WDTPW|WDTHOLD), &WDTCTL

            mov     #matriz, R12    ; Ponteiro para a matriz de entrada
            mov     #3, R13         ; Número de linhas da matriz
            mov     #3, R14         ; Número de colunas da matriz
            call    #SUM_SUB        ; Chamar sub-rotina
            jmp     $               ; Loop infinito
            nop

;-------------------------------------------------------------------------------
; R4..R7 = as quatro somas, R8 = ponteiro, R9 = linhas restantes,
; R10 = colunas restantes, R11 = elemento atual.
; Como os contadores são decrescentes:
;   1ª linha  <=> R9 == linhas     última linha  <=> R9 == 1
;   1ª coluna <=> R10 == colunas   última coluna <=> R10 == 1
;-------------------------------------------------------------------------------
SUM_SUB:
            push.w  R4
            push.w  R5
            push.w  R6
            push.w  R7
            push.w  R8
            push.w  R9
            push.w  R10
            push.w  R11
            clr.w   R4
            clr.w   R5
            clr.w   R6
            clr.w   R7
            mov.w   R12, R8
            mov.w   R13, R9
ss_linha:
            mov.w   R14, R10
ss_coluna:
            mov.w   @R8+, R11
            ; ---- somas que excluem a 1ª linha (R12 e R13)
            cmp.w   R13, R9
            jeq     ss_ultLinha     ; Está na 1ª linha
            cmp.w   R14, R10
            jeq     ss_s13          ; 1ª coluna: não entra em R12
            add.w   R11, R4
ss_s13:
            cmp.w   #1, R10
            jeq     ss_ultLinha     ; Última coluna: não entra em R13
            add.w   R11, R5
            ; ---- somas que excluem a última linha (R14 e R15)
ss_ultLinha:
            cmp.w   #1, R9
            jeq     ss_prox         ; Está na última linha
            cmp.w   R14, R10
            jeq     ss_s15          ; 1ª coluna: não entra em R14
            add.w   R11, R6
ss_s15:
            cmp.w   #1, R10
            jeq     ss_prox         ; Última coluna: não entra em R15
            add.w   R11, R7
ss_prox:
            dec.w   R10
            jnz     ss_coluna
            dec.w   R9
            jnz     ss_linha
            mov.w   R4, R12
            mov.w   R5, R13
            mov.w   R6, R14
            mov.w   R7, R15
            pop.w   R11
            pop.w   R10
            pop.w   R9
            pop.w   R8
            pop.w   R7
            pop.w   R6
            pop.w   R5
            pop.w   R4
            ret

;-------------------------------------------------------------------------------
; Especificar a matriz de entrada na seção de dados
            .data
matriz:     .word   1, 2, 3, 4, 5, 6, 7, 8, 9

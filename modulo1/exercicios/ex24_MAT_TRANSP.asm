;-------------------------------------------------------------------------------
; Módulo 1 - Exercício 24: sub-rotina MAT_TRANSP
; Entrada: R12 = ponteiro para a matriz de words (linha por linha)
;          R13 = número de linhas, R14 = número de colunas
; Escreve a transposta logo após o fim da matriz original.
;   [1 2 3; 4 5 6] em 0x2400 -> 1, 4, 2, 5, 3, 6 a partir de 0x240C
;-------------------------------------------------------------------------------
            .cdecls "msp430.h"
            .global main
            .text
main:
            mov.w   #(WDTPW|WDTHOLD), &WDTCTL

            mov     #matriz, R12    ; Ponteiro para a matriz de entrada
            mov     #2, R13         ; Número de linhas da matriz
            mov     #3, R14         ; Número de colunas da matriz
            call    #MAT_TRANSP     ; Chamar sub-rotina
            jmp     $               ; Loop infinito
            nop

;-------------------------------------------------------------------------------
; R4 = destino, R5 = tamanho de uma linha em bytes (2*colunas),
; R6 = colunas restantes, R7 = linhas restantes, R8 = origem, R9 = início da coluna
; A linha j da transposta é a coluna j da original: percorremos cada coluna
; de cima para baixo, pulando uma linha inteira (R5 bytes) a cada elemento.
;-------------------------------------------------------------------------------
MAT_TRANSP:
            push.w  R4
            push.w  R5
            push.w  R6
            push.w  R7
            push.w  R8
            push.w  R9
            mov.w   R14, R5
            rla.w   R5              ; R5 = 2 * colunas
            mov.w   R12, R4         ; Destino = início + linhas * R5
            mov.w   R13, R7
            tst.w   R7
            jz      mt_fim
            tst.w   R14
            jz      mt_fim
mt_fimMatriz:
            add.w   R5, R4
            dec.w   R7
            jnz     mt_fimMatriz
            mov.w   R12, R9         ; Coluna 0
            mov.w   R14, R6
mt_coluna:
            mov.w   R9, R8
            mov.w   R13, R7
mt_linha:
            mov.w   @R8, 0(R4)      ; T[j][i] = A[i][j]
            incd.w  R4
            add.w   R5, R8          ; Desce uma linha na original
            dec.w   R7
            jnz     mt_linha
            incd.w  R9              ; Próxima coluna
            dec.w   R6
            jnz     mt_coluna
mt_fim:
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
matriz:     .word   1, 2, 3, 4, 5, 6
            .space  12              ; Espaço reservado para a transposta

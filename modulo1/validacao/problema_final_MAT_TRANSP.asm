;-------------------------------------------------------------------------------
; Módulo 1 - Problema Final: sub-rotina MAT_TRANSP
; Entrada: R12 = ponteiro para uma matriz de words (escrita linha por linha)
;          R13 = número de linhas, R14 = número de colunas
; A transposta é escrita a partir da primeira posição após o fim da original.
;
; Exemplo: A = [1 2 3; 4 5 6]
;   0x2400..0x240A: 1 2 3 4 5 6   (continua igual)
;   0x240C..0x2416: 1 4 2 5 3 6   (transposta, 3 linhas x 2 colunas)
; Funciona para matrizes de qualquer tamanho.
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
; Registradores usados (todos salvos na pilha):
;   R4 = ponteiro de escrita (transposta)
;   R5 = tamanho de uma linha da original em bytes (2 * colunas)
;   R6 = colunas que faltam percorrer
;   R7 = linhas que faltam percorrer
;   R8 = ponteiro de leitura (elemento A[i][j])
;   R9 = endereço do topo da coluna j (A[0][j])
;
; A linha j da transposta é a coluna j da original. Para cada coluna j,
; descemos pela original somando R5 ao ponteiro e escrevemos em sequência.
;-------------------------------------------------------------------------------
MAT_TRANSP:
        push.w  R4
        push.w  R5
        push.w  R6
        push.w  R7
        push.w  R8
        push.w  R9

        mov.w   R14, R5
        rla.w   R5              ; R5 = 2 * colunas (bytes por linha)

        ; Destino = início + linhas * (2 * colunas)
        mov.w   R12, R4
        mov.w   R13, R7
        tst.w   R7              ; Matriz vazia: nada a fazer
        jz      mt_fim
        tst.w   R14
        jz      mt_fim
mt_fimMatriz:
        add.w   R5, R4
        dec.w   R7
        jnz     mt_fimMatriz

        mov.w   R12, R9         ; Começa na coluna 0
        mov.w   R14, R6
mt_coluna:
        mov.w   R9, R8          ; Topo da coluna j
        mov.w   R13, R7
mt_linha:
        mov.w   @R8, 0(R4)      ; T[j][i] = A[i][j]
        incd.w  R4              ; Próxima posição da transposta
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

matriz: .word   1, 2, 3, 4, 5, 6
        .space  12              ; Espaço para a transposta (mesmo tamanho)

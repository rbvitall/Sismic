;-------------------------------------------------------------------------------
; Módulo 1 - Exercício 23: sub-rotina ARAB_ROM (arábico -> romano)
; Entrada: R12 = número entre 1 e 3999, R13 = endereço da string de saída
; Escreve a string em algarismos romanos terminada em 0x00.
; Algoritmo guloso: subtrai o maior valor da tabela que couber e escreve o
; símbolo correspondente (com os pares subtrativos CM, CD, XC, XL, IX, IV).
;-------------------------------------------------------------------------------
            .cdecls "msp430.h"
            .global main
            .text
main:
            mov.w   #(WDTPW|WDTHOLD), &WDTCTL

            mov.w   #1479, R12
            mov.w   #saida, R13
            call    #ARAB_ROM
            jmp     $               ; saida = "MCDLXXIX"
            nop

;-------------------------------------------------------------------------------
; R4 = ponteiro para a entrada atual da tabela
;-------------------------------------------------------------------------------
ARAB_ROM:
            push.w  R4
            mov.w   #tabela, R4
ar_loop:
            tst.w   R12
            jz      ar_fim
            cmp.w   0(R4), R12      ; número - valor da tabela
            jlo     ar_proxima      ; Não cabe: tenta o próximo valor
            sub.w   0(R4), R12
            mov.b   2(R4), 0(R13)   ; Primeiro símbolo
            inc.w   R13
            tst.b   3(R4)
            jz      ar_loop         ; Símbolo simples
            mov.b   3(R4), 0(R13)   ; Segundo símbolo (par subtrativo)
            inc.w   R13
            jmp     ar_loop
ar_proxima:
            add.w   #4, R4
            jmp     ar_loop
ar_fim:
            mov.b   #0, 0(R13)      ; Terminador da string
            pop.w   R4
            ret

;-------------------------------------------------------------------------------
; Tabela (em FLASH): valor (word) + até dois símbolos (bytes)
;-------------------------------------------------------------------------------
tabela:     .word   1000
            .byte   'M', 0
            .word   900
            .byte   'C', 'M'
            .word   500
            .byte   'D', 0
            .word   400
            .byte   'C', 'D'
            .word   100
            .byte   'C', 0
            .word   90
            .byte   'X', 'C'
            .word   50
            .byte   'L', 0
            .word   40
            .byte   'X', 'L'
            .word   10
            .byte   'X', 0
            .word   9
            .byte   'I', 'X'
            .word   5
            .byte   'V', 0
            .word   4
            .byte   'I', 'V'
            .word   1
            .byte   'I', 0

;-------------------------------------------------------------------------------
            .data
saida:      .space  16              ; Maior caso: 3888 = "MMMDCCCLXXXVIII" (15+1)

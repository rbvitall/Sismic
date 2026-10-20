;-------------------------------------------------------------------------------
; Módulo 1 - Exercício 19: sub-rotina W16_ASC
; Entrada: R12 = número de 16 bits, R13 = endereço do vetor de saída
; Escreve os 4 dígitos hexadecimais em ASCII. Ex.: 0x89AB -> '8','9','A','B'
;-------------------------------------------------------------------------------
            .cdecls "msp430.h"
            .global main
            .text
main:
            mov.w   #(WDTPW|WDTHOLD), &WDTCTL

            mov.w   #0x89AB, R12
            mov.w   #saida, R13
            call    #W16_ASC
            jmp     $               ; saida = 0x38, 0x39, 0x41, 0x42
            nop

;-------------------------------------------------------------------------------
; W16_ASC: R4 = valor, R5 = ponteiro de escrita, R6 = contador de nibbles
;-------------------------------------------------------------------------------
W16_ASC:
            push.w  R4
            push.w  R5
            push.w  R6
            mov.w   R12, R4
            mov.w   R13, R5
            mov.w   #4, R6
w16_loop:
            mov.w   R4, R12         ; Nibble mais significativo -> bits 3..0
            swpb    R12
            rra.w   R12
            rra.w   R12
            rra.w   R12
            rra.w   R12
            and.w   #0x000F, R12
            call    #NIB_ASC
            mov.b   R12, 0(R5)
            inc.w   R5
            rla.w   R4              ; Próximo nibble vai para o topo
            rla.w   R4
            rla.w   R4
            rla.w   R4
            dec.w   R6
            jnz     w16_loop
            pop.w   R6
            pop.w   R5
            pop.w   R4
            ret

;-------------------------------------------------------------------------------
; NIB_ASC: R12 = nibble (0..15) -> R12 = código ASCII ('0'..'9', 'A'..'F')
;-------------------------------------------------------------------------------
NIB_ASC:
            cmp.w   #10, R12
            jlo     nib_digito
            add.w   #('A' - 10), R12
            ret
nib_digito:
            add.w   #'0', R12
            ret

;-------------------------------------------------------------------------------
            .data
saida:      .space  4

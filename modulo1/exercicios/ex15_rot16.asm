;-------------------------------------------------------------------------------
; Módulo 1 - Exercício 15: sub-rotina rot16
; Entrada: R12 = valor a rotacionar
;          R13 = opções  [15:12] direção (0 esq., 1 dir.)
;                        [11:8]  tipo (0 insere 0, 1 insere 1, 2 aritmética,
;                                      4 circular)
;                        [3:0]   número de rotações
; Saída:   R12 = valor rotacionado
;
; Cada passo prepara o Carry com o bit que deve entrar e usa RLC (esquerda)
; ou RRC (direita), que inserem o Carry no LSB/MSB.
;-------------------------------------------------------------------------------
            .cdecls "msp430.h"
            .global main
            .text
main:
            mov.w   #(WDTPW|WDTHOLD), &WDTCTL

            mov.w   #0x1234, R12    ; Circular 8 bits p/ direita: 0x3412
            mov.w   #0x1408, R13
            call    #rot16
            mov.w   R12, R4

            mov.w   #0x1234, R12    ; Lógica 14 bits p/ esquerda com 0's: 0x0000
            mov.w   #0x000E, R13
            call    #rot16
            mov.w   R12, R5

            mov.w   #0x8001, R12    ; Aritmética 3 bits p/ direita: 0xF000
            mov.w   #0x1203, R13
            call    #rot16
            mov.w   R12, R6

            mov.w   #0x0F00, R12    ; Lógica 4 bits p/ esquerda com 1's: 0xF00F
            mov.w   #0x0104, R13
            call    #rot16
            mov.w   R12, R7

            jmp     $
            nop

;-------------------------------------------------------------------------------
; rot16: R4 = N rotações, R5 = tipo
;-------------------------------------------------------------------------------
rot16:
            push.w  R4
            push.w  R5
            mov.w   R13, R4
            and.w   #0x000F, R4     ; N
            jz      rot16_fim
            mov.w   R13, R5
            swpb    R5
            and.w   #0x000F, R5     ; Tipo
rot16_loop:
            bit.w   #0xF000, R13    ; Direção: C = 1 -> direita
            jc      rot16_dir
            call    #rotEsq1
            jmp     rot16_prox
rot16_dir:
            call    #rotDir1
rot16_prox:
            dec.w   R4
            jnz     rot16_loop
rot16_fim:
            pop.w   R5
            pop.w   R4
            ret

;-------------------------------------------------------------------------------
; rotEsq1: rotaciona R12 1 bit para a esquerda conforme o tipo em R5
;-------------------------------------------------------------------------------
rotEsq1:
            cmp.w   #1, R5
            jeq     re_um
            cmp.w   #4, R5
            jeq     re_circ
            clrc                    ; Tipos 0 e 2: entra 0 no LSB
            jmp     re_faz
re_um:
            setc                    ; Tipo 1: entra 1
            jmp     re_faz
re_circ:
            bit.w   #0x8000, R12    ; Tipo 4: C = MSB atual
re_faz:
            rlc.w   R12
            ret

;-------------------------------------------------------------------------------
; rotDir1: rotaciona R12 1 bit para a direita conforme o tipo em R5
;-------------------------------------------------------------------------------
rotDir1:
            cmp.w   #1, R5
            jeq     rd_um
            cmp.w   #2, R5
            jeq     rd_arit
            cmp.w   #4, R5
            jeq     rd_circ
            clrc                    ; Tipo 0: entra 0 no MSB
            jmp     rd_faz
rd_um:
            setc                    ; Tipo 1: entra 1
            jmp     rd_faz
rd_arit:
            bit.w   #0x8000, R12    ; Tipo 2: replica o bit de sinal
            jmp     rd_faz
rd_circ:
            bit.w   #0x0001, R12    ; Tipo 4: C = LSB atual
rd_faz:
            rrc.w   R12
            ret

;-------------------------------------------------------------------------------
; Módulo 1 - Exercício 14: sub-rotina m2m4
; Entrada: R12 = endereço de um vetor de bytes, R13 = tamanho
; Saída:   R12 = quantidade de múltiplos de 2, R13 = quantidade de múltiplos de 4
; BIT faz C = 1 quando o resultado do "E" é diferente de zero. Assim:
;   bit #1  -> C = 0 se o bit de peso 1 for 0 (múltiplo de 2)
;   bit #3  -> C = 0 se os bits de peso 1 e 2 forem 0 (múltiplo de 4)
;-------------------------------------------------------------------------------
            .cdecls "msp430.h"
            .global main
            .text
main:
            mov.w   #(WDTPW|WDTHOLD), &WDTCTL

            mov.w   #vetor, R12
            mov.w   #10, R13
            call    #m2m4
            jmp     $               ; R12 = 6 (0,2,4,6,8,10), R13 = 3 (0,4,8)
            nop

;-------------------------------------------------------------------------------
m2m4:
            push.w  R4
            push.w  R5
            push.w  R6
            clr.w   R4              ; Múltiplos de 2
            clr.w   R5              ; Múltiplos de 4
            tst.w   R13
            jz      m24_fim
m24_loop:
            mov.b   @R12+, R6
            bit.b   #0x01, R6       ; Bit de peso 1 setado?
            jc      m24_prox        ; Sim: ímpar
            inc.w   R4              ; Múltiplo de 2
            bit.b   #0x03, R6       ; Bits de peso 1 e 2
            jc      m24_prox
            inc.w   R5              ; Múltiplo de 4
m24_prox:
            dec.w   R13
            jnz     m24_loop
m24_fim:
            mov.w   R4, R12
            mov.w   R5, R13
            pop.w   R6
            pop.w   R5
            pop.w   R4
            ret

;-------------------------------------------------------------------------------
            .data
vetor:      .byte   0, 1, 2, 3, 4, 6, 8, 10, 13, 255

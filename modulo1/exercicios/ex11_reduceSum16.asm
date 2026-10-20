;-------------------------------------------------------------------------------
; Módulo 1 - Exercício 11: sub-rotina reduceSum16
; Entrada: R12 = endereço do vetor de words, R13 = tamanho do vetor
; Saída:   R13:R12 = soma de 32 bits (MSWord:LSWord)
;-------------------------------------------------------------------------------
            .cdecls "msp430.h"
            .global main
            .text
main:
            mov.w   #(WDTPW|WDTHOLD), &WDTCTL

            mov.w   #vetor, R12
            mov.w   #6, R13
            call    #reduceSum16
            jmp     $               ; Soma = 200010 = 0x00030D4A
            nop

;-------------------------------------------------------------------------------
reduceSum16:
            clr.w   R14             ; LSWord
            clr.w   R15             ; MSWord
            tst.w   R13
            jz      rs16_fim
rs16_loop:
            add.w   @R12+, R14      ; Soma a parte baixa
            addc.w  #0, R15         ; Propaga o carry para a parte alta
            dec.w   R13
            jnz     rs16_loop
rs16_fim:
            mov.w   R14, R12
            mov.w   R15, R13
            ret

;-------------------------------------------------------------------------------
            .data
vetor:      .word   65535, 50000, 40000, 30000, 10, 14465

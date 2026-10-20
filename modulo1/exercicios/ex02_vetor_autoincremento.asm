;-------------------------------------------------------------------------------
; Módulo 1 - Exercício 2 (guiado): vetor na RAM e modo @Rn+
; Resultados: R5 = 0x12, R6 = 0x34, R7 = 0x7856, R8 = 0xBC9A, var8 = 0x33
; As words ficam com os bytes "trocados" porque o MSP430 é little endian
; (o byte de menor endereço é o menos significativo).
;-------------------------------------------------------------------------------
            .cdecls "msp430.h"
            .global main
            .text
main:
            mov.w   #(WDTPW|WDTHOLD), &WDTCTL

            ; a) R4 aponta para o vetor
            mov.w   #vetor, R4

            ; b) Lê os dois primeiros bytes com autoincremento (+1 em .B)
            mov.b   @R4+, R5
            mov.b   @R4+, R6

            ; c) Lê as duas próximas words (+2 em .W)
            mov.w   @R4+, R7
            mov.w   @R4+, R8

            ; d) Escreve 0x33 em var8 usando o modo absoluto (&label)
            mov.b   #0x33, &var8

            jmp     $
            nop

;-------------------------------------------------------------------------------
            .data
vetor:      .byte   0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE
var8:       .byte   0x00

;-------------------------------------------------------------------------------
; Módulo 1 - Exercício 1 (guiado): modos de endereçamento
; Resultado esperado na memória a partir de 0x2400 (visão 8 bits):
;   FE CA 34 12 78 56
;-------------------------------------------------------------------------------
            .cdecls "msp430.h"
            .global main
            .text
main:
            mov.w   #(WDTPW|WDTHOLD), &WDTCTL

            ; a) Modo imediato: R4 aponta para o início da RAM
            mov.w   #0x2400, R4

            ; a) Modo indexado: grava o byte 0xFE na posição apontada por R4
            mov.b   #0xFE, 0(R4)

            ; b) mov.b #0xFE, @R4  -> ERRO de montagem: o modo indireto (@Rn)
            ;    não pode ser usado como destino. Por isso usamos 0(R4).

            ; c) Grava 0xCA na próxima posição (R4 + 1)
            mov.b   #0xCA, 1(R4)

            ; d) Words andam de 2 em 2 (little endian: 0x1234 -> 34 12)
            mov.w   #0x1234, 2(R4)
            mov.w   #0x5678, 4(R4)

            ; e) (opcional) Word em endereço ímpar: o MSP430 ignora o bit 0 do
            ;    endereço em acessos de 16 bits, então a escrita abaixo iria
            ;    para 0x2404 (sobrescrevendo 0x5678). Deixada comentada.
            ; mov.w #0xABCD, 5(R4)

            jmp     $
            nop

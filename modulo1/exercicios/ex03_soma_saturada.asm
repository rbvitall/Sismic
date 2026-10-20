;-------------------------------------------------------------------------------
; Módulo 1 - Exercício 3: soma saturada R4 = sat(R4 + R5)
; Casos de teste (troque os valores abaixo):
;   R4 = 123,    R5 = 456    -> R4 = 579    (sem carry: salta para FIM)
;   R4 = 0xFFF0, R5 = 0x0020 -> R4 = 0xFFFF (com carry: satura)
;-------------------------------------------------------------------------------
            .cdecls "msp430.h"
            .global main
            .text
main:
            mov.w   #(WDTPW|WDTHOLD), &WDTCTL

            mov.w   #0xFFF0, R4
            mov.w   #0x0020, R5

            add.w   R5, R4          ; R4 = R4 + R5
            jnc     FIM             ; Sem carry: resultado já está certo
            mov.w   #0xFFFF, R4     ; Com carry: trava no máximo
FIM:
            jmp     $
            nop

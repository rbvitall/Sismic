;-------------------------------------------------------------------------------
; Módulo 1 - Exercício 5: múltiplas condições
; R4 = R4 + R5; se positivo soma 1, se negativo subtrai 1, se zero não muda.
; Os saltos não alteram as flags, por isso JZ e JN podem vir em sequência.
;-------------------------------------------------------------------------------
            .cdecls "msp430.h"
            .global main
            .text
main:
            mov.w   #(WDTPW|WDTHOLD), &WDTCTL

            mov.w   #-10, R4
            mov.w   #3, R5

            add.w   R5, R4          ; R4 = R4 + R5 (atualiza N e Z)
            jz      FIM             ; Zero: não muda
            jn      NEGATIVO        ; Negativo
POSITIVO:
            inc.w   R4              ; Positivo: soma 1
            jmp     FIM             ; Salto incondicional para pular o bloco seguinte
NEGATIVO:
            dec.w   R4              ; Negativo: subtrai 1
FIM:
            jmp     $               ; Ex.: -10 + 3 = -7 -> R4 = -8
            nop

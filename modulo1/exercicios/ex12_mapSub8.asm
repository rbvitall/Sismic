;-------------------------------------------------------------------------------
; Módulo 1 - Exercício 12: sub-rotina mapSub8 (s = a - b, vetores de bytes)
; Entrada: R12 = endereço de s, R13 = endereço de a, R14 = endereço de b,
;          R15 = número de elementos
; Escreve direto na memória, sem retorno.
;-------------------------------------------------------------------------------
            .cdecls "msp430.h"
            .global main
            .text
main:
            mov.w   #(WDTPW|WDTHOLD), &WDTCTL

            mov.w   #vetor_s, R12
            mov.w   #vetor_a, R13
            mov.w   #vetor_b, R14
            mov.w   #5, R15
            call    #mapSub8
            jmp     $               ; s = 9, 18, 27, 36, 0xFF (5 - 6 = -1)
            nop

;-------------------------------------------------------------------------------
mapSub8:
            push.w  R4
            tst.w   R15
            jz      ms8_fim
ms8_loop:
            mov.b   @R13+, R4       ; R4 = a[i]
            sub.b   @R14+, R4       ; R4 = a[i] - b[i]
            mov.b   R4, 0(R12)      ; s[i] = R4
            inc.w   R12
            dec.w   R15
            jnz     ms8_loop
ms8_fim:
            pop.w   R4
            ret

;-------------------------------------------------------------------------------
            .data
vetor_a:    .byte   10, 20, 30, 40, 5
vetor_b:    .byte   1, 2, 3, 4, 6
vetor_s:    .byte   0, 0, 0, 0, 0   ; Saída inicializada com zeros

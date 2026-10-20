;-------------------------------------------------------------------------------
; Módulo 1 - Exercício 17: sub-rotina maior16
; Entrada: R12 = endereço de um vetor de words sem sinal, R13 = tamanho
; Saída:   R12 = maior elemento, R13 = frequência
; Usa o mesmo vetor do exercício 16, agora lido como 5 words (little endian):
;   0x03C8, 0x032D, 0x80FF, 0x5A03, 0xFA11  -> maior = 0xFA11, freq = 1
;-------------------------------------------------------------------------------
            .cdecls "msp430.h"
            .global main
            .text
main:
            mov.w   #(WDTPW|WDTHOLD), &WDTCTL

            mov.w   #vetor, R12
            mov.w   #5, R13
            call    #maior16
            jmp     $
            nop

;-------------------------------------------------------------------------------
maior16:
            push.w  R4
            push.w  R5
            push.w  R6
            mov.w   @R12, R4        ; Maior = primeiro elemento
            clr.w   R5
maior_loop:
            mov.w   @R12+, R6
            cmp.w   R4, R6          ; R6 - R4
            jlo     maior_prox      ; R6 < maior
            jeq     maior_igual     ; R6 == maior
            mov.w   R6, R4          ; R6 > maior: novo maior
            mov.w   #1, R5
            jmp     maior_prox
maior_igual:
            inc.w   R5
maior_prox:
            dec.w   R13
            jnz     maior_loop
            mov.w   R4, R12
            mov.w   R5, R13
            pop.w   R6
            pop.w   R5
            pop.w   R4
            ret

;-------------------------------------------------------------------------------
            .data
vetor:      .byte   200, 3, 45, 3, 255, 128, 3, 90, 17, 250

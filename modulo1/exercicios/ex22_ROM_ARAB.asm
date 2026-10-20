;-------------------------------------------------------------------------------
; Módulo 1 - Exercício 22: sub-rotina ROM_ARAB (romano -> arábico)
; Entrada: R12 = endereço de uma string com algarismos romanos (termina em 0)
; Saída:   R12 = número correspondente
; Para cada algarismo: se o próximo for maior, subtrai o seu peso; senão, soma.
;   "MCDLXXIX" -> 1000 + (500-100) + (50+10+10) + (10-1) = 1479
;-------------------------------------------------------------------------------
            .cdecls "msp430.h"
            .global main
            .text
main:
            mov.w   #(WDTPW|WDTHOLD), &WDTCTL

            mov.w   #romano, R12
            call    #ROM_ARAB
            jmp     $               ; R12 = 1479
            nop

;-------------------------------------------------------------------------------
; R4 = soma, R5 = peso atual, R6 = peso do próximo
;-------------------------------------------------------------------------------
ROM_ARAB:
            push.w  R4
            push.w  R5
            push.w  R6
            clr.w   R4
            mov.b   @R12+, R13
            call    #VAL_ROM
            mov.w   R13, R5         ; Peso do primeiro algarismo
ra_loop:
            tst.w   R5
            jz      ra_fim          ; Chegou ao terminador
            mov.b   @R12+, R13
            call    #VAL_ROM
            mov.w   R13, R6         ; Peso do próximo (0 no fim da string)
            cmp.w   R6, R5          ; atual - próximo
            jhs     ra_soma         ; atual >= próximo: soma
            sub.w   R5, R4          ; atual < próximo: subtrai
            jmp     ra_avanca
ra_soma:
            add.w   R5, R4
ra_avanca:
            mov.w   R6, R5
            jmp     ra_loop
ra_fim:
            mov.w   R4, R12
            pop.w   R6
            pop.w   R5
            pop.w   R4
            ret

;-------------------------------------------------------------------------------
; VAL_ROM: R13 = caractere romano -> R13 = peso (0 se não for algarismo)
;-------------------------------------------------------------------------------
VAL_ROM:
            cmp.b   #'I', R13
            jne     vr_V
            mov.w   #1, R13
            ret
vr_V:       cmp.b   #'V', R13
            jne     vr_X
            mov.w   #5, R13
            ret
vr_X:       cmp.b   #'X', R13
            jne     vr_L
            mov.w   #10, R13
            ret
vr_L:       cmp.b   #'L', R13
            jne     vr_C
            mov.w   #50, R13
            ret
vr_C:       cmp.b   #'C', R13
            jne     vr_D
            mov.w   #100, R13
            ret
vr_D:       cmp.b   #'D', R13
            jne     vr_M
            mov.w   #500, R13
            ret
vr_M:       cmp.b   #'M', R13
            jne     vr_zero
            mov.w   #1000, R13
            ret
vr_zero:    clr.w   R13
            ret

;-------------------------------------------------------------------------------
            .data
romano:     .cstring "MCDLXXIX"

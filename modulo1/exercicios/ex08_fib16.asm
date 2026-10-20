;-------------------------------------------------------------------------------
; Módulo 1 - Exercício 8: sub-rotina FIB16
; Retorna em R12 o maior número de Fibonacci que cabe em 16 bits (46368).
; A sequência avança até a soma gerar carry (estouro de 16 bits sem sinal).
;-------------------------------------------------------------------------------
            .cdecls "msp430.h"
            .global main
            .text
main:
            mov.w   #(WDTPW|WDTHOLD), &WDTCTL

            call    #FIB16
            jmp     $               ; R12 = 46368 (0xB520)
            nop

;-------------------------------------------------------------------------------
; a = R13, b = R12, temp = R4
;-------------------------------------------------------------------------------
FIB16:
            push.w  R4
            clr.w   R13             ; a = 0
            mov.w   #1, R12         ; b = 1
FIB16_loop:
            mov.w   R12, R4         ; temp = b
            add.w   R13, R4         ; temp = a + b
            jc      FIB16_fim       ; Estourou: b é o maior que cabe
            mov.w   R12, R13        ; a = b
            mov.w   R4, R12         ; b = a + b
            jmp     FIB16_loop
FIB16_fim:
            pop.w   R4
            ret

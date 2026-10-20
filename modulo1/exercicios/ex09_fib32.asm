;-------------------------------------------------------------------------------
; Módulo 1 - Exercício 9: sub-rotina FIB32
; Retorna em R13:R12 (MSWord:LSWord) o maior número de Fibonacci que cabe em
; 32 bits: F(47) = 2971215073 = 0xB11924E1 -> R13 = 0xB119, R12 = 0x24E1
;-------------------------------------------------------------------------------
            .cdecls "msp430.h"
            .global main
            .text
main:
            mov.w   #(WDTPW|WDTHOLD), &WDTCTL

            call    #FIB32
            jmp     $
            nop

;-------------------------------------------------------------------------------
; a = R15:R14, b = R13:R12, temp = R5:R4
;-------------------------------------------------------------------------------
FIB32:
            push.w  R4
            push.w  R5
            clr.w   R14             ; a = 0
            clr.w   R15
            mov.w   #1, R12         ; b = 1
            clr.w   R13
FIB32_loop:
            mov.w   R12, R4         ; temp = b
            mov.w   R13, R5
            add.w   R14, R4         ; temp = a + b (LSWord)
            addc.w  R15, R5         ;              (MSWord + carry)
            jc      FIB32_fim       ; Estourou 32 bits: b é o maior
            mov.w   R12, R14        ; a = b
            mov.w   R13, R15
            mov.w   R4, R12         ; b = temp
            mov.w   R5, R13
            jmp     FIB32_loop
FIB32_fim:
            pop.w   R5
            pop.w   R4
            ret

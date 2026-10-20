;-------------------------------------------------------------------------------
; Módulo 1 - Exercício 7: sub-rotina FIB
; Grava a partir de 0x2400 os 20 primeiros números de Fibonacci (16 bits):
;   0, 1, 1, 2, 3, 5, 8, ..., 4181
; Entrada: R12 = endereço de destino (0x2400)
;-------------------------------------------------------------------------------
            .cdecls "msp430.h"
            .global main
            .text
main:
            mov.w   #(WDTPW|WDTHOLD), &WDTCTL

            mov.w   #0x2400, R12
            call    #FIB
            jmp     $
            nop

;-------------------------------------------------------------------------------
; FIB: R12 = ponteiro de saída. Usa R4 (a), R5 (b), R13 (contador), R14 (temp)
;-------------------------------------------------------------------------------
FIB:
            push.w  R4
            push.w  R5
            clr.w   R4              ; a = F(0) = 0
            mov.w   #1, R5          ; b = F(1) = 1
            mov.w   #20, R13        ; Quantidade de termos
FIB_loop:
            mov.w   R4, 0(R12)      ; Grava a
            incd.w  R12             ; Próxima word
            mov.w   R5, R14         ; temp = b
            add.w   R4, R5          ; b = a + b
            mov.w   R14, R4         ; a = antigo b
            dec.w   R13
            jnz     FIB_loop
            pop.w   R5
            pop.w   R4
            ret

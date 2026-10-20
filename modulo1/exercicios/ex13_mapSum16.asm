;-------------------------------------------------------------------------------
; Módulo 1 - Exercício 13: sub-rotina mapSum16 (soma elemento a elemento)
;   R5 = 0x2400 -> endereço do vetor 1
;   R6 = 0x2410 -> endereço do vetor 2
;   R7 = 0x2420 -> endereço do vetor soma
;   R8 = número de elementos
; A soma é de 16 bits (resultados maiores que 65535 "dão a volta").
;-------------------------------------------------------------------------------
            .cdecls "msp430.h"
            .global main
            .text
main:
            mov.w   #(WDTPW|WDTHOLD), &WDTCTL

            mov.w   #vetor1, R5     ; 0x2400
            mov.w   #vetor2, R6     ; 0x2410
            mov.w   #soma, R7       ; 0x2420
            mov.w   #8, R8
            call    #mapSum16
            jmp     $
            nop

;-------------------------------------------------------------------------------
mapSum16:
            push.w  R9
            tst.w   R8
            jz      msum_fim
msum_loop:
            mov.w   @R5+, R9        ; R9 = vetor1[i]
            add.w   @R6+, R9        ; R9 += vetor2[i]
            mov.w   R9, 0(R7)       ; soma[i] = R9
            incd.w  R7
            dec.w   R8
            jnz     msum_loop
msum_fim:
            pop.w   R9
            ret

;-------------------------------------------------------------------------------
            .data
vetor1:     .word   7, 65000, 50054, 26472, 53000, 60606, 814, 41121
vetor2:     .word   7, 226, 3400, 26472, 470, 1020, 44444, 12345
soma:       .space  16

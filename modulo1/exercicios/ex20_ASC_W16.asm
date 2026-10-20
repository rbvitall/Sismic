;-------------------------------------------------------------------------------
; Módulo 1 - Exercício 20: sub-rotina ASC_W16 (inversa do exercício 19)
; Entrada: R12 = endereço de um vetor com 4 códigos ASCII
; Saída:   R12 = word correspondente, Carry = 1 (sucesso) ou 0 (código inválido)
; Válidos: '0'..'9' (0x30-0x39) e 'A'..'F' (0x41-0x46)
;-------------------------------------------------------------------------------
            .cdecls "msp430.h"
            .global main
            .text
main:
            mov.w   #(WDTPW|WDTHOLD), &WDTCTL

            mov.w   #MEMO, R12
            call    #ASC_W16        ; Chamar sub-rotina
OK          jc      OK              ; Trava a execução com sucesso (R12 = 0x89AB)
NOK         jnc     NOK             ; Trava a execução com falha

;-------------------------------------------------------------------------------
; ASC_W16: R4 = resultado parcial, R5 = contador
;-------------------------------------------------------------------------------
ASC_W16:
            push.w  R4
            push.w  R5
            clr.w   R4
            mov.w   #4, R5
asc_loop:
            mov.b   @R12+, R13
            call    #ASC_NIB        ; R13 = nibble, C = válido
            jnc     asc_falha
            rla.w   R4              ; Abre espaço para o novo nibble
            rla.w   R4
            rla.w   R4
            rla.w   R4
            bis.w   R13, R4
            dec.w   R5
            jnz     asc_loop
            mov.w   R4, R12
            pop.w   R5              ; POP não altera as flags
            pop.w   R4
            setc                    ; Sucesso
            ret
asc_falha:
            pop.w   R5
            pop.w   R4
            clrc                    ; Falha
            ret

;-------------------------------------------------------------------------------
; ASC_NIB: R13 = código ASCII -> R13 = valor (0..15), C = 1 se válido
;-------------------------------------------------------------------------------
ASC_NIB:
            cmp.b   #'0', R13
            jlo     an_erro         ; < '0'
            cmp.b   #('9' + 1), R13
            jlo     an_digito       ; '0'..'9'
            cmp.b   #'A', R13
            jlo     an_erro         ; entre '9' e 'A'
            cmp.b   #('F' + 1), R13
            jhs     an_erro         ; > 'F'
            sub.w   #('A' - 10), R13
            setc
            ret
an_digito:
            sub.w   #'0', R13
            setc
            ret
an_erro:
            clrc
            ret

;-------------------------------------------------------------------------------
            .data
; Declarar 4 caracteres ASCII (0x38, 0x39, 0x41, 0x42)
MEMO:       .byte   '8', '9', 'A', 'B'

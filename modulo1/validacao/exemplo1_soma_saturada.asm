; Módulo 1 - Exemplo 1 (Listagem 1): soma saturada entre R4 e R5
; Todos os nossos programas em assembly se iniciam com as linhas abaixo

        .cdecls "msp430.h"
        .global main
        .text

main:
        mov.w #(WDTPW|WDTHOLD), &WDTCTL

        ; Inicializar R4 e R5 com números quaisquer (teste com outros)
        mov.w #123, R4
        mov.w #456, R5

        add.w R5, R4 ; Fazer a soma: R4 = R4 + R5

        jnc   FIM    ; Se não houve carry, pular para o rótulo "FIM"

        ; A linha abaixo só será executada se o salto não tiver acontecido, i.e., se houve carry
        mov.w #0xFFFF, R4

FIM:
        jmp   $      ; Loop infinito
        nop

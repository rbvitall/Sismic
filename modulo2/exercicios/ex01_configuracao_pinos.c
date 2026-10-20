// Módulo 2 - Exercício 1: Configuração dos pinos
// O LED verde (P4.7) imita o estado da chave S1 (P2.1):
// S1 pressionada -> LED aceso; S1 solta -> LED apagado.

#include <msp430.h>

#define S1   BIT1   // P2.1
#define LED2 BIT7   // P4.7 (verde)

void main(void){
    WDTCTL = WDTPW | WDTHOLD;

    P2DIR &= ~S1;       // S1 como entrada
    P2REN |= S1;        // Habilita resistor
    P2OUT |= S1;        // Pull-up (chave fechada = 0)

    P4DIR |= LED2;      // LED como saída
    P4OUT &= ~LED2;

    for(;;){
        if((P2IN & S1) == 0)    // Chave fechada
            P4OUT |= LED2;
        else                    // Chave aberta
            P4OUT &= ~LED2;
    }
}

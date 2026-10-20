// Módulo 2 - Exercício 2: Ruído (rebotes) de chaves mecânicas
// Alterna o LED vermelho (P1.0) a cada aperto de S1, SEM remover rebotes.
// Observe que às vezes o LED alterna mais de uma vez por aperto: são os
// rebotes da chave sendo contados como vários acionamentos.

#include <msp430.h>

#define S1   BIT1   // P2.1
#define LED1 BIT0   // P1.0 (vermelho)

void main(void){
    WDTCTL = WDTPW | WDTHOLD;

    P2DIR &= ~S1;
    P2REN |= S1;
    P2OUT |= S1;

    P1DIR |= LED1;
    P1OUT &= ~LED1;

    for(;;){
        while((P2IN & S1) != 0);    // Espera a chave fechar
        P1OUT ^= LED1;              // Alterna o LED
        while((P2IN & S1) == 0);    // Espera a chave abrir
    }
}

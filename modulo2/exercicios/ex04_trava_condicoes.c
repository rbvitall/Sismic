// Módulo 2 - Exercício 4: Trava de execução com condições mais elaboradas
// Alterna o LED vermelho toda vez que S1 OU S2 for acionada (aberta -> fechada).
// A trava espera até que alguma das chaves feche; depois espera as duas
// abrirem antes de aceitar um novo acionamento.

#include <msp430.h>

#define S1   BIT1   // P2.1
#define S2   BIT1   // P1.1
#define LED1 BIT0   // P1.0 (vermelho)

#define S1_FECHADA ((P2IN & S1) == 0)
#define S2_FECHADA ((P1IN & S2) == 0)

void debounce(void){
    volatile unsigned int i = 2000;
    while(i != 0)
        i--;
}

void main(void){
    WDTCTL = WDTPW | WDTHOLD;

    P2DIR &= ~S1;  P2REN |= S1;  P2OUT |= S1;   // S1 com pull-up
    P1DIR &= ~S2;  P1REN |= S2;  P1OUT |= S2;   // S2 com pull-up

    P1DIR |= LED1;
    P1OUT &= ~LED1;

    for(;;){
        while(!S1_FECHADA && !S2_FECHADA);  // Trava: espera S1 ou S2 fechar
        P1OUT ^= LED1;
        debounce();
        while(S1_FECHADA || S2_FECHADA);    // Trava: espera as duas abrirem
        debounce();
    }
}

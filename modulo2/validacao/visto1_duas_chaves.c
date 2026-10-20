// Módulo 2 - Visto 1
// Alterna o LED 1 (vermelho) sempre que S1 for acionada e o LED 2 (verde)
// sempre que S2 for acionada ("acionada" = passou de aberta para fechada),
// eliminando os rebotes.
//
// Segue o fluxograma sugerido: a cada iteração lê as chaves (X1, X2) e
// compara com o estado anterior (X1anterior, X2anterior). A CPU nunca fica
// presa esperando uma chave; só gasta o tempo de debounce quando alguma
// chave muda de estado.

#include <msp430.h>

#define ABERTO  1
#define FECHADO 0

#define S1   BIT1   // P2.1
#define S2   BIT1   // P1.1
#define LED1 BIT0   // P1.0 (vermelho)
#define LED2 BIT7   // P4.7 (verde)

void main(void){
    WDTCTL = WDTPW | WDTHOLD;

    P2DIR &= ~S1;  P2REN |= S1;  P2OUT |= S1;   // S1 com pull-up
    P1DIR &= ~S2;  P1REN |= S2;  P1OUT |= S2;   // S2 com pull-up

    P1DIR |= LED1;  P1OUT &= ~LED1;
    P4DIR |= LED2;  P4OUT &= ~LED2;

    int X1, X2;
    int X1anterior = ABERTO;    // Qualquer valor serve na primeira iteração
    int X2anterior = ABERTO;

    while(1){
        X1 = (P2IN & S1) ? ABERTO : FECHADO;
        X2 = (P1IN & S2) ? ABERTO : FECHADO;

        if(X1 == FECHADO && X1anterior == ABERTO){  // S1 acionada
            P1OUT ^= LED1;
        }

        if(X2 == FECHADO && X2anterior == ABERTO){  // S2 acionada
            P4OUT ^= LED2;
        }

        if(X1 != X1anterior || X2 != X2anterior){   // Houve mudança de estado
            __delay_cycles(20000);                  // Debounce (~20 ms)
        }

        X1anterior = X1;
        X2anterior = X2;
    }
}

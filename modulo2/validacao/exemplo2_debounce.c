// Módulo 2 - Exemplo 2: Remoção de rebotes
// Alterna o estado do LED 2 quando a chave S1 for acionada.

#include <msp430.h>

#define S1 BIT1      // P2.1 para S1
#define LED2 BIT7    // P4.7 para o LED2

void main(void){

    WDTCTL = WDTPW | WDTHOLD;

    P2DIR &= ~S1;
    P2REN |= S1;
    P2OUT |= S1;

    P4DIR |= LED2;
    P4OUT &= ~LED2;

    for(;;){

        // Avançar só quando a chave fechar
        while ((P2IN & S1) != 0);

        // Executar ação (toggle)
        P4OUT ^= LED2;

        // Debounce
        __delay_cycles(10000);

        // Avançar só quando a chave abrir
        while ((P2IN & S1) == 0);

        // Debounce
        __delay_cycles(10000);
    }
}

// Módulo 2 - Exercício 6: Amostrando flags do timer
// Pisca o LED verde (P4.7) em exatamente 1 Hz (0,5 s aceso / 0,5 s apagado)
// fazendo polling da flag CCIFG do canal 0 do Timer A0.
// ACLK = 32768 Hz -> 0,5 s = 16384 tiques -> CCR0 = 16383 (modo UP)

#include <msp430.h>

#define LED2 BIT7   // P4.7 (verde)

void main(void){
    WDTCTL = WDTPW | WDTHOLD;

    P4DIR |= LED2;
    P4OUT &= ~LED2;

    TA0CCR0 = 16384 - 1;                        // 0,5 s
    TA0CTL = TASSEL__ACLK | MC__UP | TACLR;     // ACLK, modo UP

    for(;;){
        while((TA0CCTL0 & CCIFG) == 0);         // Espera o contador chegar em CCR0
        TA0CCTL0 &= ~CCIFG;                     // Limpa a flag (sem ISR é manual)
        P4OUT ^= LED2;
    }
}

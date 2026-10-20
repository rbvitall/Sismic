// Módulo 2 - Exercício 14: [Timer] Interrupção dedicada
// Repete o exercício 6 (LED verde a 1 Hz) usando a interrupção do CCR0.
// A flag CCIFG do canal 0 é limpa automaticamente quando a ISR é atendida.

#include <msp430.h>

#define LED2 BIT7   // P4.7 (verde)

void main(void){
    WDTCTL = WDTPW | WDTHOLD;

    P4DIR |= LED2;
    P4OUT &= ~LED2;

    TA0CCR0 = 16384 - 1;                        // 0,5 s com ACLK
    TA0CCTL0 = CCIE;                            // Interrupção dedicada do CCR0
    TA0CTL = TASSEL__ACLK | MC__UP | TACLR;

    __enable_interrupt();

    for(;;){
        __low_power_mode_3();
    }
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void TA0_CCR0_ISR(void){
    P4OUT ^= LED2;
}

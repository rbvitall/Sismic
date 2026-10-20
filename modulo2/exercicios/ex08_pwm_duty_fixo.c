// Módulo 2 - Exercício 8: PWM com duty cycle fixo
// LED vermelho piscando a 128 Hz com 50% de duty cycle, por software, usando
// as interrupções de overflow (TAIFG) e do canal 1 (CCR1) do Timer A0.
// ACLK = 32768 Hz -> 32768 / 128 = 256 tiques por período (CCR0 = 255)
//   TAIFG (contador volta a 0) -> acende o LED
//   CCR1 = 128 (metade)        -> apaga o LED

#include <msp430.h>

#define LED1 BIT0   // P1.0 (vermelho)

void main(void){
    WDTCTL = WDTPW | WDTHOLD;

    P1DIR |= LED1;
    P1OUT &= ~LED1;

    TA0CCR0 = 256 - 1;                                  // Período: 1/128 s
    TA0CCR1 = 128;                                      // 50%
    TA0CCTL1 = CCIE;                                    // Interrupção do CCR1
    TA0CTL = TASSEL__ACLK | MC__UP | TACLR | TAIE;      // + interrupção de overflow

    __enable_interrupt();

    for(;;){
        __low_power_mode_3();           // Dorme; o ACLK continua rodando
    }
}

// Interrupções agrupadas do Timer A0 (CCR1..CCR4 e overflow)
#pragma vector = TIMER0_A1_VECTOR
__interrupt void TA0_CCRN_ISR(void){
    switch(TA0IV){                      // A leitura de TA0IV limpa a flag atendida
        case TA0IV_TACCR1:              // Meio do período
            P1OUT &= ~LED1;
            break;
        case TA0IV_TAIFG:               // Início do período
            P1OUT |= LED1;
            break;
        default:
            break;
    }
}

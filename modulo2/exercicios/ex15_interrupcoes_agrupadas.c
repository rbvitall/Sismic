// Módulo 2 - Exercício 15: [Timer] Interrupções agrupadas
// PWM por software com as interrupções agrupadas do Timer A0:
//   LED vermelho com 30% de duty cycle (controlado pelo CCR1)
//   LED verde    com 70% de duty cycle (controlado pelo CCR2)
// Período de 100 tiques do ACLK (~328 Hz, sem cintilação visível).
// No overflow (TAIFG) os dois LEDs acendem; cada CCRn apaga o seu LED.

#include <msp430.h>

#define LED1 BIT0   // P1.0 vermelho
#define LED2 BIT7   // P4.7 verde

#define PERIODO 100

void main(void){
    WDTCTL = WDTPW | WDTHOLD;

    P1DIR |= LED1;  P1OUT &= ~LED1;
    P4DIR |= LED2;  P4OUT &= ~LED2;

    TA0CCR0 = PERIODO - 1;
    TA0CCR1 = (PERIODO * 30) / 100;             // 30%
    TA0CCR2 = (PERIODO * 70) / 100;             // 70%
    TA0CCTL1 = CCIE;
    TA0CCTL2 = CCIE;
    TA0CTL = TASSEL__ACLK | MC__UP | TACLR | TAIE;

    __enable_interrupt();

    for(;;){
        __low_power_mode_3();
    }
}

#pragma vector = TIMER0_A1_VECTOR
__interrupt void TA0_CCRN_ISR(void){
    switch(TA0IV){                  // TA0IV indica a fonte de maior prioridade
        case TA0IV_TACCR1:          // 30% do período: apaga o vermelho
            P1OUT &= ~LED1;
            break;
        case TA0IV_TACCR2:          // 70% do período: apaga o verde
            P4OUT &= ~LED2;
            break;
        case TA0IV_TAIFG:           // Novo período: acende os dois
            P1OUT |= LED1;
            P4OUT |= LED2;
            break;
        default:
            break;
    }
}

// Módulo 2 - Exercício 9: PWM por hardware
// Mesmo sinal do exercício 8 (128 Hz, 50%), gerado pela própria saída do timer.
// Pelo datasheet do MSP430F5529, a saída do canal 1 do Timer A0 (TA0.1) está
// no pino P1.2. Ligação: retire o jumper JP8 e ligue P1.2 ao pino de BAIXO do
// jumper (o mais próximo da borda da placa). Não ligue no outro pino!
//
// Modo de saída 7 (Reset/Set): a saída vai a 0 quando TA0R = CCR1 e a 1
// quando TA0R = CCR0. A CPU não participa: não há interrupções.

#include <msp430.h>

#define TA0_1 BIT2  // P1.2 = TA0.1

void main(void){
    WDTCTL = WDTPW | WDTHOLD;

    P1DIR |= TA0_1;                     // Pino como saída
    P1SEL |= TA0_1;                     // Função alternativa: saída do timer

    TA0CCR0 = 256 - 1;                  // 128 Hz com ACLK
    TA0CCR1 = 128;                      // 50%
    TA0CCTL1 = OUTMOD_7;                // Reset/Set
    TA0CTL = TASSEL__ACLK | MC__UP | TACLR;

    for(;;){
        __low_power_mode_3();
    }
}

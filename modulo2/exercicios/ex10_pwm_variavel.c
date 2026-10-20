// Módulo 2 - Exercício 10: PWM variável
// PWM por hardware em P1.2 (TA0.1, ligado ao LED vermelho como no exercício 9).
// S2 aumenta e S1 diminui o duty cycle em passos de 12,5% (1/8) de CCR0.
// Período de 256 tiques -> passo de 32 tiques; duty vai de 0% a 100%.
// As chaves são lidas sem travar a CPU (estado atual x estado anterior).

#include <msp430.h>
#include <stdint.h>

#define ABERTO  1
#define FECHADO 0

#define S1    BIT1  // P2.1
#define S2    BIT1  // P1.1
#define TA0_1 BIT2  // P1.2

#define PERIODO 256
#define PASSO   (PERIODO / 8)       // 12,5%

void main(void){
    WDTCTL = WDTPW | WDTHOLD;

    P2DIR &= ~S1;  P2REN |= S1;  P2OUT |= S1;
    P1DIR &= ~S2;  P1REN |= S2;  P1OUT |= S2;

    P1DIR |= TA0_1;
    P1SEL |= TA0_1;

    uint16_t duty = PERIODO / 2;            // Começa em 50%

    TA0CCR0 = PERIODO - 1;
    TA0CCR1 = duty;                         // CCR1 > CCR0 -> saída sempre em 1 (100%)
    TA0CCTL1 = OUTMOD_7;
    TA0CTL = TASSEL__ACLK | MC__UP | TACLR;

    int X1, X2;
    int X1anterior = ABERTO;
    int X2anterior = ABERTO;

    while(1){
        X1 = (P2IN & S1) ? ABERTO : FECHADO;
        X2 = (P1IN & S2) ? ABERTO : FECHADO;

        if(X1 == FECHADO && X1anterior == ABERTO){      // S1 acionada: diminui
            if(duty >= PASSO)
                duty -= PASSO;
            TA0CCR1 = duty;
        }

        if(X2 == FECHADO && X2anterior == ABERTO){      // S2 acionada: aumenta
            if(duty <= PERIODO - PASSO)
                duty += PASSO;
            TA0CCR1 = duty;
        }

        if(X1 != X1anterior || X2 != X2anterior){       // Houve mudança: debounce
            __delay_cycles(20000);
        }

        X1anterior = X1;
        X2anterior = X2;
    }
}

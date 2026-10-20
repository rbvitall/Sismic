// Módulo 2 - Exercício 11: Usando um servo motor (também é o Visto 2)
// PWM de 20 ms (50 Hz) no pino P2.0 (TA1.1). A largura do pulso varia de
// 0,5 ms a 2,5 ms em passos de 0,1 ms: S2 (P1.1) aumenta e S1 (P2.1) diminui,
// como no exercício 10.
// Conexão do servo: marrom = GND, vermelho = +5 V, laranja = P2.0 (sinal PWM).
//
// Observação: no LaunchPad F5529, P1.1 é a chave S2 e P2.1 é a chave S1,
// então aqui X1 corresponde a S2 e X2 corresponde a S1.

#include <msp430.h>
#include <stdint.h>

#define ABERTO 1
#define FECHADO 0
#define MOTOR BIT0

void set_servo_pulse(uint16_t width_us) {
    TA1CCR1 = width_us - 1;
}

void main(void) {
    WDTCTL = WDTPW | WDTHOLD;

    P1DIR &= ~BIT1;             // S2 (P1.1) como entrada com pull-up
    P1REN |= BIT1;
    P1OUT |= BIT1;

    P2DIR &= ~BIT1;             // S1 (P2.1) como entrada com pull-up
    P2REN |= BIT1;
    P2OUT |= BIT1;

    P2DIR |= MOTOR;             // P2.0 como saída
    P2SEL |= MOTOR;             // Função alternativa do pino (TA1.1)

    TA1CTL = TASSEL__SMCLK | MC__UP;    // SMCLK (~1 MHz) e modo UP
    TA1CCR0 = 20000 - 1;                // Período de 20 ms
    TA1CCTL1 = OUTMOD_7;                // TA1.1 no modo Reset/Set

    uint16_t pulso_atual = 1500;        // 1,5 ms = 90 graus
    set_servo_pulse(pulso_atual);

    int X1, X2;
    int X1anterior = ABERTO;
    int X2anterior = ABERTO;

    while(1) {
        X1 = (P1IN & BIT1) ? ABERTO : FECHADO;     // S2
        X2 = (P2IN & BIT1) ? ABERTO : FECHADO;     // S1

        if (X1 == FECHADO && X1anterior == ABERTO) {
            if (pulso_atual < 2500) {       // S2 acionada: +0,1 ms
                pulso_atual += 100;
                set_servo_pulse(pulso_atual);
            }
        }

        if (X2 == FECHADO && X2anterior == ABERTO) {
            if (pulso_atual > 500) {        // S1 acionada: -0,1 ms
                pulso_atual -= 100;
                set_servo_pulse(pulso_atual);
            }
        }

        if (X1 != X1anterior || X2 != X2anterior) {
            __delay_cycles(20000);          // Debounce
        }

        X1anterior = X1;
        X2anterior = X2;
    }
}

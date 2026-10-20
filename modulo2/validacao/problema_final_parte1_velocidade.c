// Módulo 2 - Problema Final - Parte 1: Controle de velocidade do servomotor
//
// 1. Começa em 0 grau, direção positiva, velocidade 10 graus/s.
// 2. Enquanto S1 estiver fechada, a posição anda 1 grau por passo no sentido
//    da direção atual (10 ou 20 passos por segundo). S1 aberta: não gira.
// 3. Ao chegar em 180 graus (direção positiva) ou 0 grau (negativa), a direção
//    é invertida.
// 4. A velocidade alterna entre 10 e 20 graus/s quando S2 é acionada
//    (passa de FECHADA para ABERTA), com debounce.
//
// Hardware:
//   Servo: marrom = GND, vermelho = 5 V, laranja = P2.0 (TA1.1)
//   PWM de 20 ms; pulso de 0,5 ms (0 grau) a 2,5 ms (180 graus)
//
// Estrutura:
//   Timer A1 -> PWM do servo (SMCLK ~1 MHz, como no código base do Visto 2)
//   Timer A0 -> "tique" a cada 50 ms (ACLK). A ISR lê as chaves e move o servo.
//   Ler as chaves a cada 50 ms já elimina os rebotes (que duram poucos ms):
//   uma mudança só é vista depois que o sinal estabilizou.
//   O main fica em baixo consumo (LPM0 mantém o SMCLK para o PWM).

#include <msp430.h>
#include <stdint.h>

#define ABERTO  1
#define FECHADO 0

#define S1    BIT1  // P2.1
#define S2    BIT1  // P1.1
#define MOTOR BIT0  // P2.0 = TA1.1

#define TIQUE_ACLK 1638     // 50 ms com ACLK = 32768 Hz

volatile int16_t angulo = 0;            // 0..180 graus
volatile int8_t  direcao = +1;          // +1 ou -1
volatile uint8_t velocidade = 10;       // 10 ou 20 graus/s

void servo_angulo(int16_t graus){
    // 0 grau -> 500 us, 180 graus -> 2500 us: 500 + graus * 2000/180
    uint16_t pulso_us = 500 + ((uint32_t)graus * 2000UL) / 180;
    TA1CCR1 = pulso_us - 1;
}

void main(void){
    WDTCTL = WDTPW | WDTHOLD;

    P2DIR &= ~S1;  P2REN |= S1;  P2OUT |= S1;
    P1DIR &= ~S2;  P1REN |= S2;  P1OUT |= S2;

    P2DIR |= MOTOR;
    P2SEL |= MOTOR;

    // PWM do servo: período de 20 ms
    TA1CCR0 = 20000 - 1;
    TA1CCTL1 = OUTMOD_7;
    servo_angulo(angulo);                       // Vai para 0 grau
    TA1CTL = TASSEL__SMCLK | MC__UP | TACLR;

    // Base de tempo de 50 ms
    TA0CCR0 = TIQUE_ACLK - 1;
    TA0CCTL0 = CCIE;
    TA0CTL = TASSEL__ACLK | MC__UP | TACLR;

    __enable_interrupt();

    for(;;){
        __low_power_mode_0();
    }
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void TA0_CCR0_ISR(void){
    static int X2anterior = ABERTO;
    static uint8_t tiques = 0;

    int X1 = (P2IN & S1) ? ABERTO : FECHADO;
    int X2 = (P1IN & S2) ? ABERTO : FECHADO;

    // S2 acionada = passou de fechada para aberta: troca a velocidade
    if(X2 == ABERTO && X2anterior == FECHADO){
        velocidade = (velocidade == 10) ? 20 : 10;
    }
    X2anterior = X2;

    // 20 graus/s -> 1 passo a cada tique (50 ms)
    // 10 graus/s -> 1 passo a cada 2 tiques (100 ms)
    tiques++;
    if(tiques < (velocidade == 20 ? 1 : 2))
        return;
    tiques = 0;

    if(X1 == FECHADO){
        angulo += direcao;
        if(angulo >= 180){              // Chegou ao fim: inverte a direção
            angulo = 180;
            direcao = -1;
        }
        else if(angulo <= 0){
            angulo = 0;
            direcao = +1;
        }
        servo_angulo(angulo);
    }
}

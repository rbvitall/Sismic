// Módulo 2 - Problema Final - Parte 2: Função One-Touch
//
// 1. Com S1 pressionada, o ângulo diminui 1 grau por passo a 10 graus/s (até 0).
// 2. Com S2 pressionada, o ângulo aumenta 1 grau por passo a 10 graus/s (até 180).
// 3. S1 mantida por >= 3 s e solta: continua descendo até 0 grau ou até
//    qualquer chave ser acionada.
// 4. S2 mantida por >= 3 s e solta: continua subindo até 180 graus ou até
//    qualquer chave ser acionada.
// 5. Mantida por menos de 3 s e solta: para quando a chave é solta.
//
// Hardware: servo em P2.0 (TA1.1), PWM de 20 ms, pulso de 0,5 ms a 2,5 ms.
//
// Estrutura: o Timer A0 gera um tique a cada 100 ms (= 1 grau a 10 graus/s).
// A cada tique a ISR lê as chaves e executa uma máquina de estados. A leitura
// periódica a cada 100 ms também serve de debounce.
//
//   PARADO --S1--> MANUAL_DESCE --solta (<3 s)--> PARADO
//                               --solta (>=3 s)--> AUTO_DESCE --0 grau ou chave--> PARADO
//   PARADO --S2--> MANUAL_SOBE  (análogo, até 180 graus)
//
// Quando uma chave interrompe o movimento automático, o sistema espera as
// duas chaves serem soltas antes de aceitar um novo comando; assim o mesmo
// aperto que parou o motor não começa outro movimento.

#include <msp430.h>
#include <stdint.h>

#define S1    BIT1  // P2.1
#define S2    BIT1  // P1.1
#define MOTOR BIT0  // P2.0 = TA1.1

#define TIQUE_ACLK   3277   // 100 ms com ACLK = 32768 Hz
#define TIQUES_3S    30     // 3 s / 100 ms

#define S1_FECHADA ((P2IN & S1) == 0)
#define S2_FECHADA ((P1IN & S2) == 0)

typedef enum { PARADO, MANUAL_DESCE, MANUAL_SOBE, AUTO_DESCE, AUTO_SOBE } Estado;

volatile int16_t angulo = 90;

void servo_angulo(int16_t graus){
    uint16_t pulso_us = 500 + ((uint32_t)graus * 2000UL) / 180;
    TA1CCR1 = pulso_us - 1;
}

// Move 1 grau no sentido indicado. Retorna 1 se chegou ao limite.
uint8_t passo(int8_t sentido){
    angulo += sentido;
    if(angulo <= 0)   { angulo = 0;   servo_angulo(angulo); return 1; }
    if(angulo >= 180) { angulo = 180; servo_angulo(angulo); return 1; }
    servo_angulo(angulo);
    return 0;
}

void main(void){
    WDTCTL = WDTPW | WDTHOLD;

    P2DIR &= ~S1;  P2REN |= S1;  P2OUT |= S1;
    P1DIR &= ~S2;  P1REN |= S2;  P1OUT |= S2;

    P2DIR |= MOTOR;
    P2SEL |= MOTOR;

    TA1CCR0 = 20000 - 1;
    TA1CCTL1 = OUTMOD_7;
    servo_angulo(angulo);
    TA1CTL = TASSEL__SMCLK | MC__UP | TACLR;

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
    static Estado estado = PARADO;
    static uint16_t tempoPressionada = 0;   // Em tiques de 100 ms
    static uint8_t esperaSoltar = 0;

    uint8_t s1 = S1_FECHADA;
    uint8_t s2 = S2_FECHADA;

    switch(estado){
        case PARADO:
            if(esperaSoltar){
                if(!s1 && !s2)
                    esperaSoltar = 0;
            }
            else if(s1){
                estado = MANUAL_DESCE;
                tempoPressionada = 1;               // Este tique já conta
                passo(-1);
            }
            else if(s2){
                estado = MANUAL_SOBE;
                tempoPressionada = 1;
                passo(+1);
            }
            break;

        case MANUAL_DESCE:
            if(s1){
                if(tempoPressionada < TIQUES_3S)
                    tempoPressionada++;
                passo(-1);                          // Para sozinho em 0 grau
            }
            else{                                   // Soltou S1
                estado = (tempoPressionada >= TIQUES_3S) ? AUTO_DESCE : PARADO;
            }
            break;

        case MANUAL_SOBE:
            if(s2){
                if(tempoPressionada < TIQUES_3S)
                    tempoPressionada++;
                passo(+1);
            }
            else{
                estado = (tempoPressionada >= TIQUES_3S) ? AUTO_SOBE : PARADO;
            }
            break;

        case AUTO_DESCE:
        case AUTO_SOBE:
            if(s1 || s2){                           // Alguma chave acionada: para
                estado = PARADO;
                esperaSoltar = 1;
            }
            else if(passo(estado == AUTO_DESCE ? -1 : +1)){
                estado = PARADO;                    // Chegou ao limite
            }
            break;
    }
}

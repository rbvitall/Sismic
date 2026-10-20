// Módulo 2 - Exercício 17: Utilizando o sensor de proximidade HC-SR04
// LED vermelho aceso: objeto a menos de 15 cm.
// LED verde aceso:    objeto a mais de 30 cm.
// Entre 15 e 30 cm os dois ficam apagados.
//
// Ligações:
//   VCC -> 5V, GND -> GND
//   Trig -> P1.3 (GPIO)
//   Echo -> divisor resistivo -> P2.0 (TA1.1, entrada de captura CCI1A)
//
// Perguntas do roteiro:
// 1) Maior distância medível: o eco dura no máximo 12 ms e o som faz ida e
//    volta, então d = v*t/2 = 343 m/s * 12 ms / 2 ~ 2,06 m.
// 2) Divisor: Echo -> 4k7 -> P2.0 -> 10k -> GND. O de 10k vai ao TERRA:
//    V = 5 * 10/(10+4,7) ~ 3,4 V (nível alto válido). Invertendo (4k7 ao
//    terra) teríamos ~1,6 V, que não é reconhecido como nível alto.
//
// O Timer A1 (SMCLK, contínuo) captura as duas bordas do eco; a largura do
// pulso em us dividida por 58 dá a distância em cm.

#include <msp430.h>
#include <stdint.h>

#define LED1 BIT0   // P1.0 vermelho
#define LED2 BIT7   // P4.7 verde
#define TRIG BIT3   // P1.3
#define ECHO BIT0   // P2.0 (TA1.1)

volatile uint16_t inicioEco;
volatile uint16_t larguraEco;       // em tiques do SMCLK
volatile uint8_t  ecoPronto = 0;

void main(void){
    WDTCTL = WDTPW | WDTHOLD;

    P1DIR |= LED1;  P1OUT &= ~LED1;
    P4DIR |= LED2;  P4OUT &= ~LED2;
    P1DIR |= TRIG;  P1OUT &= ~TRIG;

    P2DIR &= ~ECHO;
    P2SEL |= ECHO;                          // TA1.1 (CCI1A)

    TA1CCTL1 = CM_3 | CCIS_0 | SCS | CAP | CCIE;    // Captura nas duas bordas
    TA1CTL = TASSEL__SMCLK | MC__CONTINUOUS | TACLR;

    __enable_interrupt();

    for(;;){
        ecoPronto = 0;

        P1OUT |= TRIG;                      // Pulso de disparo >= 10 us
        __delay_cycles(15);
        P1OUT &= ~TRIG;

        __delay_cycles(60000);              // ~60 ms: tempo para o eco voltar

        if(ecoPronto){
            uint32_t us = ((uint32_t)larguraEco * 15625UL) >> 14;   // tiques -> us
            uint16_t cm = us / 58;

            if(cm < 15)  P1OUT |= LED1; else P1OUT &= ~LED1;
            if(cm > 30)  P4OUT |= LED2; else P4OUT &= ~LED2;
        }
    }
}

#pragma vector = TIMER1_A1_VECTOR
__interrupt void TA1_CCRN_ISR(void){
    switch(TA1IV){
        case TA1IV_TACCR1:
            if(TA1CCTL1 & CCI){             // Entrada em 1: foi borda de subida
                inicioEco = TA1CCR1;
            } else {                        // Borda de descida: fim do eco
                larguraEco = TA1CCR1 - inicioEco;
                ecoPronto = 1;
            }
            break;
        default:
            break;
    }
}

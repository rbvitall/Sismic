// Módulo 3 - Exercício 12: gatilho pelo canal 1 do Timer A0 (SHS = 1, SHP = 0)
// Mesmo objetivo do exercício 11 (um eixo, 8 bits, a cada 100 ms), mas sem o
// timer interno do ADC: com SHP = 0 a amostragem dura enquanto o sinal SHI
// (aqui, a saída TA0.1) está em 1 e a conversão começa na descida.
//
// Timer A0 com SMCLK/2 (~524 kHz, ~1,9 us por tique), modo UP:
//   CCR0 = 52428  -> período de 100 ms
//   OUTMOD_7      -> TA0.1 sobe em CCR0 e desce em CCR1
//   CCR1 = 3      -> pulso de 4 tiques ~ 7,6 us (> 1,77 us do ex09)
// A saída TA0.1 é ligada internamente ao ADC; não precisa de pino.

#include <msp430.h>
#include <stdint.h>

#define N_AMOSTRAS 64

volatile uint8_t amostras[N_AMOSTRAS];
volatile uint8_t indice = 0;

void main(void){
    WDTCTL = WDTPW | WDTHOLD;

    P6SEL |= BIT0;                                  // A0

    ADC12CTL0 &= ~ADC12ENC;
    ADC12CTL0 = ADC12ON;                            // SHT não é usado com SHP = 0
    ADC12CTL1 = ADC12SHS_1 | ADC12SSEL_3 | ADC12CONSEQ_2;   // Gatilho TA0.1, repetido
    ADC12CTL2 = ADC12RES_0;                         // 8 bits
    ADC12MCTL0 = ADC12INCH_0;
    ADC12IE = ADC12IE0;
    ADC12CTL0 |= ADC12ENC;

    TA0CCR0 = 52429 - 1;                            // 100 ms com SMCLK/2
    TA0CCR1 = 3;
    TA0CCTL1 = OUTMOD_7;
    TA0CTL = TASSEL__SMCLK | ID__2 | MC__UP | TACLR;

    __enable_interrupt();

    for(;;){
        __low_power_mode_0();
    }
}

#pragma vector = ADC12_VECTOR
__interrupt void ADC12_ISR(void){
    switch(ADC12IV){
        case ADC12IV_ADC12IFG0:
            amostras[indice] = ADC12MEM0;
            indice = (indice + 1) % N_AMOSTRAS;
            break;
        default:
            break;
    }
}

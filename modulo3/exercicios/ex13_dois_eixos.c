// Módulo 3 - Exercício 13: leitura dos dois eixos do joystick
// Modo sequencial repetitivo (CONSEQ = 3) com MEM0 = A0 (eixo X, P6.0) e
// MEM1 = A1 (eixo Y, P6.1); EOS no último canal. Gatilho pelo TA0.1 como no
// exercício 12, sem MSC: cada borda do gatilho converte UM canal da
// sequência. Por isso o timer roda a cada 50 ms, e cada eixo é amostrado a
// cada 100 ms. Os dois resultados são guardados quando a sequência termina
// (interrupção de MEM1).

#include <msp430.h>
#include <stdint.h>

#define N_AMOSTRAS 64

volatile uint8_t eixoX[N_AMOSTRAS];
volatile uint8_t eixoY[N_AMOSTRAS];
volatile uint8_t indice = 0;

void main(void){
    WDTCTL = WDTPW | WDTHOLD;

    P6SEL |= BIT0 | BIT1;                           // A0 e A1

    ADC12CTL0 &= ~ADC12ENC;
    ADC12CTL0 = ADC12ON;
    ADC12CTL1 = ADC12CSTARTADD_0 | ADC12SHS_1 | ADC12SSEL_3 | ADC12CONSEQ_3;
    ADC12CTL2 = ADC12RES_0;                         // 8 bits
    ADC12MCTL0 = ADC12INCH_0;                       // X
    ADC12MCTL1 = ADC12INCH_1 | ADC12EOS;            // Y, fim da sequência
    ADC12IE = ADC12IE1;
    ADC12CTL0 |= ADC12ENC;

    TA0CCR0 = 26214 - 1;                            // 50 ms com SMCLK/2
    TA0CCR1 = 3;                                    // ~7,6 us de amostragem
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
        case ADC12IV_ADC12IFG1:                     // Sequência completa
            eixoX[indice] = ADC12MEM0;
            eixoY[indice] = ADC12MEM1;
            indice = (indice + 1) % N_AMOSTRAS;
            break;
        default:
            break;
    }
}

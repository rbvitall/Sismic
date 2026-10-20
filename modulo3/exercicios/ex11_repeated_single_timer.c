// Módulo 3 - Exercício 11: modo repeated single channel com timer
// Amostra um eixo do joystick (P6.0 / A0) a cada 100 ms, com 8 bits.
// - ADC12: CONSEQ = 2 (repetir o mesmo canal), SHP = 1, SHT0 = 0 (ver ex09)
// - Com MSC = 0 cada conversão espera um novo gatilho: a ISR do Timer A0
//   (100 ms) dispara a conversão escrevendo ADC12SC.
// - A ISR do ADC guarda o resultado no vetor circular "amostras".
// Para plotar: coloque "amostras" na aba Expressions, clique com o botão
// direito e escolha "Graph Selected Value".
//
// Joystick: pino +5V do módulo no 3V3 da placa (NUNCA no 5V), GND no GND,
// VRx -> P6.0, VRy -> P6.1.

#include <msp430.h>
#include <stdint.h>

#define N_AMOSTRAS 64

volatile uint8_t amostras[N_AMOSTRAS];
volatile uint8_t indice = 0;

void main(void){
    WDTCTL = WDTPW | WDTHOLD;

    P6SEL |= BIT0;                                  // A0

    ADC12CTL0 &= ~ADC12ENC;
    ADC12CTL0 = ADC12SHT0_0 | ADC12ON;              // 4 ciclos (> 1,77 us)
    ADC12CTL1 = ADC12SHP | ADC12SHS_0 | ADC12SSEL_3 | ADC12CONSEQ_2;
    ADC12CTL2 = ADC12RES_0;                         // 8 bits
    ADC12MCTL0 = ADC12INCH_0;
    ADC12IE = ADC12IE0;
    ADC12CTL0 |= ADC12ENC;

    TA0CCR0 = 3277 - 1;                             // 100 ms com ACLK
    TA0CCTL0 = CCIE;
    TA0CTL = TASSEL__ACLK | MC__UP | TACLR;

    __enable_interrupt();

    for(;;){
        __low_power_mode_0();                       // SMCLK é o clock do ADC
    }
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void TA0_CCR0_ISR(void){
    ADC12CTL0 |= ADC12SC;                           // Nova amostra
}

#pragma vector = ADC12_VECTOR
__interrupt void ADC12_ISR(void){
    switch(ADC12IV){
        case ADC12IV_ADC12IFG0:
            amostras[indice] = ADC12MEM0;           // Ler MEM0 limpa a flag
            indice = (indice + 1) % N_AMOSTRAS;
            ADC12CTL0 &= ~ADC12SC;
            break;
        default:
            break;
    }
}

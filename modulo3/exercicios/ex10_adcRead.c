// Módulo 3 - Exercício 10: leitura pontual de um pino da porta P6
// uint16_t adcRead(uint8_t pin): converte o canal A<pin> (P6.<pin>) uma vez.
// - 12 bits, modo single-channel single-conversion, gatilho por ADC12SC
// - Pior caso de sensor com 50 kOhm: t_sample > 12,5 us (ver ex09) ->
//   SHT0 = 2 (16 ciclos de SMCLK ~ 16 us)
// O main lê P6.0 e P6.1 em sequência; veja "leitura0" e "leitura1" no CCS.

#include <msp430.h>
#include <stdint.h>

volatile uint16_t leitura0, leitura1;

uint16_t adcRead(uint8_t pin){
    ADC12CTL0 &= ~ADC12ENC;                         // Para configurar
    P6SEL |= (1 << pin);                            // Pino como entrada analógica

    ADC12CTL0 = ADC12SHT0_2 | ADC12ON;              // 16 ciclos de amostragem, liga o ADC
    ADC12CTL1 = ADC12SHP | ADC12SHS_0 |             // Timer interno, gatilho ADC12SC
                ADC12SSEL_3 | ADC12CONSEQ_0;        // SMCLK, canal único / conversão única
    ADC12CTL2 = ADC12RES_2;                         // 12 bits
    ADC12MCTL0 = ADC12SREF_0 | pin;                 // Referência AVCC/AVSS, canal A<pin>
    ADC12IFG = 0;

    ADC12CTL0 |= ADC12ENC | ADC12SC;                // Habilita e inicia
    while(!(ADC12IFG & ADC12IFG0));                 // Espera a conversão
    return ADC12MEM0;                               // Ler MEM0 limpa a flag
}

void main(void){
    WDTCTL = WDTPW | WDTHOLD;

    for(;;){
        leitura0 = adcRead(0);
        leitura1 = adcRead(1);
        __delay_cycles(100000);                     // Breakpoint aqui
    }
}

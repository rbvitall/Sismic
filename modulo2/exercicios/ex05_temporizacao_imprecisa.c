// Módulo 2 - Exercício 5: Temporização imprecisa
// Pisca o LED vermelho em aproximadamente 1 Hz (500 ms aceso, 500 ms apagado)
// usando apenas laços de programa. O tempo real depende de quantos ciclos o
// código gerado pelo compilador gasta por iteração, por isso é impreciso:
// ajuste ATRASO_500MS medindo com um cronômetro/osciloscópio.

#include <msp430.h>
#include <stdint.h>

#define LED1 BIT0   // P1.0 (vermelho)

// Com MCLK ~1,048 MHz, cada iteração do laço com variável volatile de 32 bits
// gasta ~10 ciclos -> 524288 ciclos / 10 ~ 52000 iterações em 500 ms
#define ATRASO_500MS 52000UL

void atraso(void){
    volatile uint32_t i;
    for(i = 0; i < ATRASO_500MS; i++);
}

void main(void){
    WDTCTL = WDTPW | WDTHOLD;

    P1DIR |= LED1;
    P1OUT &= ~LED1;

    for(;;){
        P1OUT ^= LED1;
        atraso();
    }
}

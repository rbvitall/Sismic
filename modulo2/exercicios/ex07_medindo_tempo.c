// Módulo 2 - Exercício 7: Medindo tempo
// Mede o tempo de execução das duas linhas pedidas usando o Timer A0 em modo
// contínuo com SMCLK (~1,048576 MHz, ~0,954 us por tique). Lê o contador antes
// e depois do trecho; a diferença (em tiques) é convertida para microssegundos.
// Coloque um breakpoint no laço final e veja "tiques" e "tempo_us" no CCS.

#include <msp430.h>
#include <stdint.h>
#include <math.h>

volatile uint16_t tiques;
volatile uint32_t tempo_us;

void main(void){
    WDTCTL = WDTPW | WDTHOLD;

    TA0CTL = TASSEL__SMCLK | MC__CONTINUOUS | TACLR;

    uint16_t inicio = TA0R;

    volatile double hardVar = 128.43984610923f;
    hardVar = (sqrt(hardVar * 3.14159265359) + 30.3245)/1020.2331556 - 0.11923;

    uint16_t fim = TA0R;

    // Subtração em 16 bits já trata uma eventual volta do contador
    // (vale enquanto o trecho durar menos de 65536 tiques, ~62 ms)
    tiques = fim - inicio;

    // tempo = tiques / 1,048576 MHz = tiques * 10^6 / 2^20 = tiques * 15625 / 16384
    tempo_us = ((uint32_t)tiques * 15625UL) >> 14;

    for(;;){
        __no_operation();               // Breakpoint aqui
    }
}

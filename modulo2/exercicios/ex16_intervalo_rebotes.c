// Módulo 2 - Exercício 16: Medir o intervalo entre os rebotes de uma chave
// O Timer A1 em modo de captura registra cada borda (subida e descida) da
// chave S1. No F5529, P2.1 (S1) é também a entrada CCI2A do Timer A1 (TA1.2),
// então basta selecionar a função alternativa do pino.
//
// - Clock do timer: SMCLK (~1,048576 MHz) -> ~0,95 us por tique.
// - Cada captura gera um carimbo de tempo de 32 bits (estouros:CCR2).
// - A janela de 1 s começa na primeira borda (acionamento de S1) e é contada
//   pelo Timer A0 com o ACLK. Ao final, os intervalos são convertidos para
//   microssegundos e ficam no vetor "rebotes" (consulte no CCS com breakpoint).

#include <msp430.h>
#include <stdint.h>

#define S1 BIT1             // P2.1 = TA1.2 (CCI2A)
#define MAX_REBOTES 64

volatile uint32_t rebotes[MAX_REBOTES];     // Intervalos entre bordas (us)
volatile uint16_t nRebotes = 0;
volatile uint8_t  medindo = 0;
volatile uint8_t  terminou = 0;

volatile uint16_t estouros = 0;             // Parte alta do carimbo de tempo
volatile uint32_t ultimaBorda;

void main(void){
    WDTCTL = WDTPW | WDTHOLD;

    P2DIR &= ~S1;
    P2REN |= S1;
    P2OUT |= S1;                            // Pull-up
    P2SEL |= S1;                            // Entrada de captura do Timer A1

    // Timer A1: captura nas duas bordas, entrada CCI2A, síncrona
    TA1CCTL2 = CM_3 | CCIS_0 | SCS | CAP | CCIE;
    TA1CTL = TASSEL__SMCLK | MC__CONTINUOUS | TACLR | TAIE;

    __enable_interrupt();

    while(!terminou){
        __low_power_mode_0();               // SMCLK precisa continuar ligado
    }

    // Converte tiques para microssegundos: us = tiques * 10^6 / 2^20
    uint16_t i;
    for(i = 0; i < nRebotes; i++){
        rebotes[i] = (uint32_t)(((uint64_t)rebotes[i] * 15625ULL) >> 14);
    }

    for(;;){
        __no_operation();                   // Breakpoint: veja "rebotes" e "nRebotes"
    }
}

// Interrupções agrupadas do Timer A1: captura do CCR2 e overflow
#pragma vector = TIMER1_A1_VECTOR
__interrupt void TA1_CCRN_ISR(void){
    switch(TA1IV){
        case TA1IV_TACCR2: {
            uint16_t cap = TA1CCR2;
            uint16_t alta = estouros;
            // Overflow aconteceu mas ainda não foi atendido (CCR2 tem prioridade
            // maior): se a captura é "pequena", ela é posterior ao overflow
            if((TA1CTL & TAIFG) && cap < 0x8000)
                alta++;
            uint32_t agora = ((uint32_t)alta << 16) | cap;

            if(!medindo){                   // Primeira borda: abre a janela de 1 s
                medindo = 1;
                TA0CCR0 = 32768 - 1;
                TA0CCTL0 = CCIE;
                TA0CTL = TASSEL__ACLK | MC__UP | TACLR;
            }
            else if(nRebotes < MAX_REBOTES){
                rebotes[nRebotes++] = agora - ultimaBorda;
            }
            ultimaBorda = agora;
            break;
        }
        case TA1IV_TAIFG:
            estouros++;
            break;
        default:
            break;
    }
}

// Fim da janela de 1 s
#pragma vector = TIMER0_A0_VECTOR
__interrupt void TA0_CCR0_ISR(void){
    TA0CTL = MC__STOP;
    TA1CCTL2 &= ~CCIE;                      // Para de capturar
    TA1CTL &= ~TAIE;
    terminou = 1;
    __low_power_mode_off_on_exit();         // Acorda o main
}

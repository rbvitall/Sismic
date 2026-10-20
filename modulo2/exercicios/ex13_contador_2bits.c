// Módulo 2 - Exercício 13: [GPIO] Múltiplas interrupções
// Contador binário de 2 bits mostrado nos LEDs (verde = LSB, vermelho = MSB).
// S1 incrementa e S2 decrementa; os LEDs começam apagados.
//
// Debounce sem travar a CPU: ao detectar uma borda, a interrupção daquela
// chave é desligada e o Timer A1 conta 20 ms. Na ISR do timer a chave é
// "rearmada" esperando a borda contrária (se está fechada, espera abrir e
// vice-versa). Só a borda de fechamento (descida) altera o contador.

#include <msp430.h>
#include <stdint.h>

#define S1   BIT1   // P2.1
#define S2   BIT1   // P1.1
#define LED1 BIT0   // P1.0 vermelho (MSB)
#define LED2 BIT7   // P4.7 verde (LSB)

volatile uint8_t contador = 0;

void mostrar(void){
    if(contador & 0x01) P4OUT |= LED2; else P4OUT &= ~LED2;
    if(contador & 0x02) P1OUT |= LED1; else P1OUT &= ~LED1;
}

void iniciarDebounce(void){
    TA1CCR0 = 655;                              // ~20 ms com ACLK (32768 Hz)
    TA1CCTL0 = CCIE;
    TA1CTL = TASSEL__ACLK | MC__UP | TACLR;
}

void main(void){
    WDTCTL = WDTPW | WDTHOLD;

    P1DIR |= LED1;  P4DIR |= LED2;
    mostrar();

    P2DIR &= ~S1;  P2REN |= S1;  P2OUT |= S1;
    P1DIR &= ~S2;  P1REN |= S2;  P1OUT |= S2;

    P2IES |= S1;  P2IFG &= ~S1;  P2IE |= S1;   // Borda de descida
    P1IES |= S2;  P1IFG &= ~S2;  P1IE |= S2;

    __enable_interrupt();

    for(;;){
        __low_power_mode_3();
    }
}

// S1
#pragma vector = PORT2_VECTOR
__interrupt void PORT2_ISR(void){
    if(P2IV == P2IV_P2IFG1){
        if(P2IES & S1)                      // Era borda de descida: fechou
            contador = (contador + 1) & 0x03;
        mostrar();
        P2IE &= ~S1;                        // Ignora rebotes até o timer
        iniciarDebounce();
    }
}

// S2
#pragma vector = PORT1_VECTOR
__interrupt void PORT1_ISR(void){
    if(P1IV == P1IV_P1IFG1){
        if(P1IES & S2)
            contador = (contador - 1) & 0x03;
        mostrar();
        P1IE &= ~S2;
        iniciarDebounce();
    }
}

// Fim dos 20 ms: rearma as chaves que estavam desabilitadas
#pragma vector = TIMER1_A0_VECTOR
__interrupt void TA1_CCR0_ISR(void){
    TA1CTL = MC__STOP;

    if(!(P2IE & S1)){
        if(P2IN & S1) P2IES |= S1;          // Aberta: espera fechar
        else          P2IES &= ~S1;         // Fechada: espera abrir
        P2IFG &= ~S1;                       // Mudar IES pode setar a flag
        P2IE |= S1;
    }
    if(!(P1IE & S2)){
        if(P1IN & S2) P1IES |= S2;
        else          P1IES &= ~S2;
        P1IFG &= ~S2;
        P1IE |= S2;
    }
}

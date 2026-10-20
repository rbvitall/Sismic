// Módulo 2 - Exercício 12: [GPIO] Configuração de interrupções
// Alterna o LED vermelho ao pressionar S1, usando a interrupção da porta P2.
// A interrupção é gerada na borda de descida de P2.1 (chave fechando).
// Um pequeno atraso dentro da ISR descarta os rebotes antes de limpar a flag.

#include <msp430.h>

#define S1   BIT1   // P2.1
#define LED1 BIT0   // P1.0

void main(void){
    WDTCTL = WDTPW | WDTHOLD;

    P1DIR |= LED1;
    P1OUT &= ~LED1;

    P2DIR &= ~S1;
    P2REN |= S1;
    P2OUT |= S1;        // Pull-up
    P2IES |= S1;        // Borda de descida (aberta -> fechada)
    P2IFG &= ~S1;       // Limpa flag que possa ter sido setada na configuração
    P2IE  |= S1;        // Habilita a interrupção do pino

    __enable_interrupt();

    for(;;){
        __low_power_mode_4();           // Dorme até a interrupção
    }
}

#pragma vector = PORT2_VECTOR
__interrupt void PORT2_ISR(void){
    switch(P2IV){                       // Ler P2IV limpa a flag de maior prioridade
        case P2IV_P2IFG1:               // S1
            P1OUT ^= LED1;
            __delay_cycles(20000);      // Debounce do fechamento
            while((P2IN & S1) == 0);    // Espera soltar
            __delay_cycles(20000);      // Debounce da abertura
            P2IFG &= ~S1;               // Descarta bordas geradas pelos rebotes
            break;
        default:
            break;
    }
}

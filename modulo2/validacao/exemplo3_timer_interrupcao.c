// Módulo 2 - Exemplo 3: Temporizadores e interrupções
// Alterna o estado do LED 2 a cada 10 segundos usando a interrupção do Timer A0.

#include <msp430.h>

#define LED2 BIT7    // P4.7 para o LED2

// Configurar o Timer A0 para gerar uma interrupção a cada 10 segundos
void init_TimerA0(void){

    // Com ACLK = 2^15 Hz
    // Para 10 segundos, precisamos contar 10 * 2^15 tiques, mas o limite é 2^16
    // Usamos ID_3 (dividir por 2^3 = 8) para reduzir o número de tiques
    // Total de tiques = 10 * 2^15 / 8 = 40960

    TA0CCR0 = 40959;                // Contar de 0 a 40959 (40960 tiques)
    TA0CCTL0 = CCIE;                // Habilitar interrupção de CCR0

    // ACLK, dividir por 8, UP mode
    TA0CTL = TASSEL__ACLK | ID_3 | MC__UP;
}

// Interrupt Service Routine (ISR) do Timer A0 CCR0
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A0_ISR(void){

    P4OUT ^= LED2;
}

void main(void){
    // Travar watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    // CONFIGURAR PINOS
    P4DIR |= LED2;  // Configurar o pino do LED como saída
    P4OUT &= ~LED2; // Começar apagado

    // OUTRAS CONFIGURAÇÕES

    init_TimerA0();                 // Configurar Timer A0

    __enable_interrupt();           // Habilitar interrupções globais

    // LOOP INFINITO

    for(;;){                        // Apenas espera a interrupção
        __no_operation();
    }
}

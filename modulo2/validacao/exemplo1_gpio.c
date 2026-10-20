// Módulo 2 - Exemplo 1: Configuração dos pinos de GPIO
// LED 2 aceso enquanto a chave S1 estiver fechada e apagado enquanto aberta.

#include <msp430.h>  // Sempre inclua esta biblioteca

#define S1 BIT1      // P2.1 para S1
#define LED2 BIT7    // P4.7 para o LED2

void main(void){

    // Travar o watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    // CONFIGURAR PINOS

    P2DIR &= ~S1; // Configurar S1 como entrada
    P2REN |= S1;  // Habilitar resistor para S1
    P2OUT |= S1;  // Configurar resistor de S1 como pull-up

    P4DIR |= LED2;  // Configurar o pino do LED como saída
    P4OUT &= ~LED2; // Começar apagado

    // LOOP INFINITO

    for(;;){

        // Se a chave estiver fechada (entrada == 0)
        if((P2IN & S1) == 0){

            P4OUT |= LED2; // Acender LED
        }

        // Se a chave estiver aberta (entrada != 0)
        else{

            P4OUT &= ~LED2; // Apagar LED
        }
    }
}

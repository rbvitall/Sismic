// Módulo 3 - Visto 3: mensagens do MSP430 para o computador via UART
// Sempre que a chave S1 for acionada, a frase é escrita no terminal do CCS.
//
// A USCI_A1 (P4.4 = TXD, P4.5 = RXD) é ligada internamente ao eZ-FET, que
// aparece no PC como "MSP Application UART1". No CCS: View -> Terminal ->
// Serial Terminal, porta COM correspondente, 9600 bps, 8 bits, sem paridade,
// 1 stop bit (ver exercício 19 do Modulo3.pdf).
//
// Baud rate com SMCLK = 1048576 Hz:
//   1048576 / 9600 = 109,23 -> UCBR = 109, UCBRS = round(0,23 * 8) = 2

#include <msp430.h>
#include <stdint.h>

#define ABERTO  1
#define FECHADO 0

#define S1 BIT1     // P2.1

void initUART_A1(void){
    UCA1CTL1 = UCSWRST;                 // Reset para configurar
    UCA1CTL0 = 0;                       // UART 8N1, LSB primeiro
    UCA1CTL1 = UCSSEL__SMCLK | UCSWRST;
    UCA1BR0 = 109;
    UCA1BR1 = 0;
    UCA1MCTL = UCBRS_2 | UCBRF_0;
    P4SEL |= BIT4 | BIT5;               // P4.4 = TXD, P4.5 = RXD
    UCA1CTL1 &= ~UCSWRST;
}

void uartPrint(char *str){
    while(*str){
        while(!(UCA1IFG & UCTXIFG));    // Espera o buffer de transmissão
        UCA1TXBUF = *str++;
    }
}

void main(void){
    WDTCTL = WDTPW | WDTHOLD;

    P2DIR &= ~S1;  P2REN |= S1;  P2OUT |= S1;

    initUART_A1();

    int X1;
    int X1anterior = ABERTO;

    while(1){
        X1 = (P2IN & S1) ? ABERTO : FECHADO;

        if(X1 == FECHADO && X1anterior == ABERTO){      // S1 acionada
            uartPrint("Ola do MSP430! Sistemas Microprocessados 2026\r\n");
        }

        if(X1 != X1anterior){
            __delay_cycles(20000);                      // Debounce
        }

        X1anterior = X1;
    }
}

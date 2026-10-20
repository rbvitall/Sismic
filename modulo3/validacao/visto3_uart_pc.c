// ============================================================================
// Lab. Sistemas Microprocessados 2026.2 - Modulo 3 - VISTO 3
// A cada acionamento de S1, envia uma frase ao terminal serial do CCS
// MSP430F5529 LaunchPad - USCI_A1 (P4.4 = TX, P4.5 = RX, ligacao interna eZ-FET)
//
// Terminal do CCS (View -> Console -> Serial Console -> nova conexao):
//   Porta: "MSP Application UART1" (COMx no Gerenciador de Dispositivos)
//   9600 baud, 8 bits de dados, sem paridade, 1 stop bit
// ============================================================================
#include <msp430.h>
#include <stdint.h>

#define FRASE "Visto 3 - MSP430 falando com o PC via UART!\r\n"

void initUART_A1(void);
void initChaveS1(void);
void uartPrint(char* str);

void main(void) {
    WDTCTL = WDTPW | WDTHOLD;

    initUART_A1();
    initChaveS1();

    uartPrint("UART pronta. Pressione S1.\r\n");

    for(;;){
        // S1 = P2.1, ativa em nivel baixo (pull-up)
        if (!(P2IN & BIT1)) {                 // Chave fechou?
            __delay_cycles(20000);            // Debounce (~20 ms)
            if (!(P2IN & BIT1)) {             // Confirmou o acionamento
                uartPrint(FRASE);             // Envia UMA vez por acionamento
                while (!(P2IN & BIT1));       // Espera soltar
                __delay_cycles(20000);        // Debounce da soltura
            }
        }
    }
}

// ----------------------------------------------------------------------------
// USCI_A1 em UART, 9600 bps, SMCLK = 1.048.576 Hz
// Tabela do User's Guide (SLAU208): UCBRx = 6, UCBRSx = 0, UCBRFx = 13, UCOS16 = 1
// ----------------------------------------------------------------------------
void initUART_A1(void) {
    UCA1CTL1 |= UCSWRST;                        // Reset para configurar
    UCA1CTL1 |= UCSSEL_2;                       // SMCLK
    UCA1BR0 = 6;
    UCA1BR1 = 0;
    UCA1MCTL = UCBRS_0 | UCBRF_13 | UCOS16;     // Oversampling
    // UCA1CTL0 = 0 (padrao): 8 bits, sem paridade, 1 stop, LSB primeiro

    P4SEL |= BIT4 | BIT5;                       // P4.4 = TXD, P4.5 = RXD
    UCA1CTL1 &= ~UCSWRST;                       // Libera a UART
}

// ----------------------------------------------------------------------------
// S1 (P2.1) como entrada com pull-up
// ----------------------------------------------------------------------------
void initChaveS1(void) {
    P2DIR &= ~BIT1;
    P2REN |=  BIT1;
    P2OUT |=  BIT1;
}

// ----------------------------------------------------------------------------
// Envia string terminada em '\0' pela USCI_A1
// ----------------------------------------------------------------------------
void uartPrint(char* str) {
    while (*str != '\0') {
        while (!(UCA1IFG & UCTXIFG));   // Espera buffer de TX livre
        UCA1TXBUF = *str++;
    }
}

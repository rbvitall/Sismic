// Módulo 3 - Exercício 17: controlar os LEDs de outro MSP430 pela serial
// Ligação (Figura 20): P3.3 (TX) de uma placa no P3.4 (RX) da outra e
// vice-versa, e os GNDs das duas placas interligados.
// O mesmo programa roda nas duas placas (cada uma é transmissora e receptora).
//
// Transmissor (apenas um código por acionamento):
//   S1 -> envia 0x01 e 0x0A alternadamente
//   S2 -> envia 0x10 e 0xA0 alternadamente
// Receptor:
//   0x01 liga o LED vermelho, 0x0A apaga o vermelho
//   0x10 liga o LED verde,    0xA0 apaga o verde
//
// UART USCI_A0, 1200 bps, ACLK. Recepção por interrupção.

#include <msp430.h>
#include <stdint.h>

#define ABERTO  1
#define FECHADO 0

#define S1   BIT1   // P2.1
#define S2   BIT1   // P1.1
#define LED1 BIT0   // P1.0 vermelho
#define LED2 BIT7   // P4.7 verde

void initUART_A0(void){
    UCA0CTL1 = UCSWRST;
    UCA0CTL0 = 0;
    UCA0CTL1 = UCSSEL__ACLK | UCSWRST;
    UCA0BR0 = 27;
    UCA0BR1 = 0;
    UCA0MCTL = UCBRS_2 | UCBRF_0;
    P3SEL |= BIT3 | BIT4;
    UCA0CTL1 &= ~UCSWRST;
    UCA0IE |= UCRXIE;                       // Interrupção de recepção
}

void uartSend(uint8_t b){
    while(!(UCA0IFG & UCTXIFG));
    UCA0TXBUF = b;
}

void main(void){
    WDTCTL = WDTPW | WDTHOLD;

    P2DIR &= ~S1;  P2REN |= S1;  P2OUT |= S1;
    P1DIR &= ~S2;  P1REN |= S2;  P1OUT |= S2;
    P1DIR |= LED1;  P1OUT &= ~LED1;
    P4DIR |= LED2;  P4OUT &= ~LED2;

    initUART_A0();
    __enable_interrupt();

    uint8_t ligarVermelho = 1;              // Próximo código de S1: 0x01
    uint8_t ligarVerde = 1;                 // Próximo código de S2: 0x10

    int X1, X2;
    int X1anterior = ABERTO;
    int X2anterior = ABERTO;

    while(1){
        X1 = (P2IN & S1) ? ABERTO : FECHADO;
        X2 = (P1IN & S2) ? ABERTO : FECHADO;

        if(X1 == FECHADO && X1anterior == ABERTO){
            uartSend(ligarVermelho ? 0x01 : 0x0A);
            ligarVermelho = !ligarVermelho;
        }
        if(X2 == FECHADO && X2anterior == ABERTO){
            uartSend(ligarVerde ? 0x10 : 0xA0);
            ligarVerde = !ligarVerde;
        }

        if(X1 != X1anterior || X2 != X2anterior){
            __delay_cycles(20000);          // Debounce
        }

        X1anterior = X1;
        X2anterior = X2;
    }
}

#pragma vector = USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void){
    switch(UCA0IV){
        case USCI_UCRXIFG:
            switch(UCA0RXBUF){
                case 0x01: P1OUT |= LED1;  break;
                case 0x0A: P1OUT &= ~LED1; break;
                case 0x10: P4OUT |= LED2;  break;
                case 0xA0: P4OUT &= ~LED2; break;
                default: break;
            }
            break;
        default:
            break;
    }
}

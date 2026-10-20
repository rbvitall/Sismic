// Módulo 3 - Exercício 15: teste da USCI_A0 em loopback
// Ligue P3.3 (TX) a P3.4 (RX) com um fio. Um byte é enviado e conferido na
// recepção por polling da flag UCRXIFG. Se o byte voltou igual, o LED verde
// acende; se não, o vermelho. Repete a cada ~0,5 s com valores diferentes.
//
// UART 1200 bps, 8N1, clock ACLK (32768 Hz):
//   N = 32768 / 1200 = 27,31 -> UCBR = 27, UCBRS = round(0,31 * 8) = 2

#include <msp430.h>
#include <stdint.h>

#define LED1 BIT0   // P1.0 vermelho
#define LED2 BIT7   // P4.7 verde

volatile uint8_t enviado, recebido;

void initUART_A0(void){
    UCA0CTL1 = UCSWRST;                     // Reset para configurar
    UCA0CTL0 = 0;                           // 8N1, LSB primeiro, UART
    UCA0CTL1 = UCSSEL__ACLK | UCSWRST;
    UCA0BR0 = 27;
    UCA0BR1 = 0;
    UCA0MCTL = UCBRS_2 | UCBRF_0;           // Modulação (sem oversampling)
    P3SEL |= BIT3 | BIT4;                   // P3.3 = TXD, P3.4 = RXD
    UCA0CTL1 &= ~UCSWRST;
}

void main(void){
    WDTCTL = WDTPW | WDTHOLD;

    P1DIR |= LED1;  P1OUT &= ~LED1;
    P4DIR |= LED2;  P4OUT &= ~LED2;

    initUART_A0();

    enviado = 0x55;
    for(;;){
        while(!(UCA0IFG & UCTXIFG));        // Buffer de transmissão livre
        UCA0TXBUF = enviado;

        while(!(UCA0IFG & UCRXIFG));        // Espera o byte voltar
        recebido = UCA0RXBUF;               // Breakpoint aqui para conferir

        if(recebido == enviado){
            P4OUT |= LED2;  P1OUT &= ~LED1;
        } else {
            P1OUT |= LED1;  P4OUT &= ~LED2;
        }

        enviado++;
        __delay_cycles(500000);
    }
}

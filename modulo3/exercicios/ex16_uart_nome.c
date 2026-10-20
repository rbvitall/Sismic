// Módulo 3 - Exercício 16: enviar e receber o nome pela serial (loopback)
// Mesma ligação e configuração do exercício 15 (P3.3 ligado a P3.4, 1200 bps).
// O nome é transmitido caractere por caractere e cada caractere recebido é
// guardado em nome_rx. O terminador '\0' também é enviado e encerra tanto a
// transmissão quanto a recepção. Ao final, compare nome_tx e nome_rx no CCS.

#include <msp430.h>
#include <stdint.h>

char nome_tx[] = "Joaquim Jose";
char nome_rx[] = "xxxxxxxxxxxx";

void initUART_A0(void){
    UCA0CTL1 = UCSWRST;
    UCA0CTL0 = 0;
    UCA0CTL1 = UCSSEL__ACLK | UCSWRST;
    UCA0BR0 = 27;                           // 1200 bps com ACLK
    UCA0BR1 = 0;
    UCA0MCTL = UCBRS_2 | UCBRF_0;
    P3SEL |= BIT3 | BIT4;
    UCA0CTL1 &= ~UCSWRST;
}

void main(void){
    WDTCTL = WDTPW | WDTHOLD;

    initUART_A0();

    uint8_t i = 0;
    char c;
    do{
        c = nome_tx[i];
        while(!(UCA0IFG & UCTXIFG));
        UCA0TXBUF = c;                      // Envia

        while(!(UCA0IFG & UCRXIFG));
        nome_rx[i] = UCA0RXBUF;             // Recebe o mesmo caractere
        i++;
    } while(c != '\0' && i < sizeof(nome_rx));

    for(;;){
        __no_operation();                   // Breakpoint: nome_rx == nome_tx
    }
}

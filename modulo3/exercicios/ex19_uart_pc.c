// Módulo 3 - Exercício 19: UART com o computador (USCI_A1 / eZ-FET)
// A USCI_A1 (P4.4 = TXD, P4.5 = RXD) está ligada dentro do LaunchPad ao
// conversor USB/serial: não é preciso nenhum fio. No PC use a porta
// "MSP Application UART1" no Serial Console do CCS com 9600 bps, 8N1.
//
// - uartPrint(str): envia uma string.
// - Recepção por interrupção, gravando num buffer circular de 32 caracteres.
// - O main devolve (eco) o que chega, para dar para ver no terminal.
//
// Baud rate com SMCLK = 1048576 Hz, mesma configuração do Visto 3 (testada
// na placa). Tabela do User's Guide (SLAU208), com oversampling:
//   UCBRx = 6, UCBRSx = 0, UCBRFx = 13, UCOS16 = 1

#include <msp430.h>
#include <stdint.h>

#define TAM_BUFFER 32

volatile char bufferRx[TAM_BUFFER];
volatile uint8_t inicio = 0;            // Próximo a ser lido
volatile uint8_t fim = 0;               // Próxima posição livre

void initUART_A1(void){
    UCA1CTL1 = UCSWRST;
    UCA1CTL0 = 0;                       // 8N1
    UCA1CTL1 = UCSSEL__SMCLK | UCSWRST;
    UCA1BR0 = 6;
    UCA1BR1 = 0;
    UCA1MCTL = UCBRS_0 | UCBRF_13 | UCOS16;     // Oversampling
    P4SEL |= BIT4 | BIT5;               // P4.4 = TXD, P4.5 = RXD
    UCA1CTL1 &= ~UCSWRST;
    UCA1IE |= UCRXIE;
}

void uartPutc(char c){
    while(!(UCA1IFG & UCTXIFG));
    UCA1TXBUF = c;
}

void uartPrint(char *str){
    while(*str)
        uartPutc(*str++);
}

// Retorna 1 e o caractere em *c se havia algo no buffer
uint8_t uartGetc(char *c){
    if(inicio == fim)
        return 0;
    *c = bufferRx[inicio];
    inicio = (inicio + 1) % TAM_BUFFER;
    return 1;
}

void main(void){
    WDTCTL = WDTPW | WDTHOLD;

    initUART_A1();
    __enable_interrupt();

    uartPrint("MSP430 pronto! Digite algo:\r\n");

    for(;;){
        char c;
        while(uartGetc(&c)){
            if(c == '\r')
                uartPrint("\r\n");
            else
                uartPutc(c);            // Eco
        }
        // Dorme até chegar um caractere. Testar o buffer com as interrupções
        // desligadas evita dormir com um caractere que chegou logo antes.
        __disable_interrupt();
        if(inicio == fim)
            __bis_SR_register(LPM0_bits | GIE);
        else
            __enable_interrupt();
    }
}

#pragma vector = USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void){
    switch(UCA1IV){
        case USCI_UCRXIFG: {
            char c = UCA1RXBUF;
            uint8_t proximo = (fim + 1) % TAM_BUFFER;
            if(proximo != inicio){      // Buffer cheio: descarta o caractere
                bufferRx[fim] = c;
                fim = proximo;
            }
            __low_power_mode_off_on_exit();
            break;
        }
        default:
            break;
    }
}

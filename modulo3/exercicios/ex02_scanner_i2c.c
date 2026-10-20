// Módulo 3 - Exercício 2: Scanner de endereços I2C
// Varre os endereços 1..127 e guarda no vetor "enderecos" os que responderam
// com ACK. Requisitar START e STOP ao mesmo tempo gera uma transação só com
// o endereço (o byte de dados não é enviado). Implementação por polling.
// Veja o resultado no CCS (variáveis "enderecos" e "nEnderecos").

#include <msp430.h>
#include <stdint.h>

uint8_t enderecos[128];
volatile uint8_t nEnderecos;

void initI2C_Master(void){
    UCB0CTL1 |= UCSWRST;
    UCB0CTL0 = UCMST | UCMODE_3 | UCSYNC;
    UCB0CTL1 = UCSSEL_2 | UCSWRST;
    UCB0BR0 = 10;                           // ~100 kHz
    UCB0BR1 = 0;

    P3SEL |= BIT0 | BIT1;
    P3DIR &= ~(BIT0 | BIT1);
    P3REN |= BIT0 | BIT1;
    P3OUT |= BIT0 | BIT1;

    UCB0CTL1 &= ~UCSWRST;
}

// Retorna 1 se algum escravo respondeu ao endereço
uint8_t i2cProbe(uint8_t addr){
    UCB0I2CSA = addr;
    UCB0IFG &= ~UCNACKIFG;
    UCB0CTL1 |= UCTR | UCTXSTT | UCTXSTP;   // START + endereço + STOP
    while(UCB0CTL1 & UCTXSTP);              // Espera o STOP
    uint8_t ack = (UCB0IFG & UCNACKIFG) ? 0 : 1;
    UCB0IFG &= ~UCNACKIFG;
    return ack;
}

// Escreve em addrs os endereços encontrados e retorna quantos foram
uint8_t i2cScan(uint8_t *addrs){
    uint8_t n = 0;
    uint8_t addr;
    for(addr = 1; addr < 128; addr++){
        if(i2cProbe(addr))
            addrs[n++] = addr;
    }
    return n;
}

void main(void){
    WDTCTL = WDTPW | WDTHOLD;

    initI2C_Master();
    nEnderecos = i2cScan(enderecos);        // LCD: 0x27 ou 0x3F

    for(;;){
        __no_operation();                   // Breakpoint aqui
    }
}

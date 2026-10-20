// Módulo 3 - Exercícios 7 e 8: leitura do LCD e bit de ocupado (busy flag)
//   Ex. 7: lcdReadNibble / lcdReadByte
//   Ex. 8: lcdBusy()
//
// Para ler, o PCF8574 precisa deixar D7-D4 em nível alto (seus pinos são
// quase-bidirecionais: escrevendo 1, o LCD consegue puxar a linha para 0).
// Com R/W = 1 e EN = 1 o LCD coloca o dado no barramento; a leitura I2C é
// feita no meio do pulso de EN, como pede o roteiro.
//
// O programa escreve "Ola" e depois lê o endereço do cursor (deve ser 3) e
// o caractere de volta da DDRAM. Veja as variáveis no CCS.

#include <msp430.h>
#include <stdint.h>

#define LCD_I2C_ADDR 0x27

#define LCD_RS 0x01
#define LCD_RW 0x02
#define LCD_EN 0x04
#define LCD_BL 0x08
#define LCD_DADOS 0xF0          // D7-D4

volatile uint8_t enderecoCursor;
volatile uint8_t ocupado;
volatile uint8_t caractereLido;

void initI2C_Master(void){
    UCB0CTL1 |= UCSWRST;
    UCB0CTL0 = UCMST | UCMODE_3 | UCSYNC;
    UCB0CTL1 = UCSSEL_2 | UCSWRST;
    UCB0BR0 = 10;
    UCB0BR1 = 0;

    P3SEL |= BIT0 | BIT1;
    P3DIR &= ~(BIT0 | BIT1);
    P3REN |= BIT0 | BIT1;
    P3OUT |= BIT0 | BIT1;

    UCB0CTL1 &= ~UCSWRST;
}

uint8_t i2cSend(uint8_t addr, uint8_t data){
    UCB0I2CSA = addr;
    UCB0CTL1 |= UCTR | UCTXSTT;
    while(!(UCB0IFG & UCTXIFG));
    UCB0TXBUF = data;
    while(UCB0CTL1 & UCTXSTT);

    uint8_t nack = (UCB0IFG & UCNACKIFG) ? 1 : 0;
    if(!nack)
        while(!(UCB0IFG & UCTXIFG));

    UCB0CTL1 |= UCTXSTP;
    while(UCB0CTL1 & UCTXSTP);
    UCB0IFG &= ~UCNACKIFG;
    return nack;
}

// Lê um byte do escravo (mestre receptor)
uint8_t i2cRead(uint8_t addr){
    UCB0I2CSA = addr;
    UCB0CTL1 &= ~UCTR;                      // Receptor
    UCB0CTL1 |= UCTXSTT;                    // START + endereço (R/W = 1)
    while(UCB0CTL1 & UCTXSTT);              // Espera o ACK do endereço
    UCB0CTL1 |= UCTXSTP;                    // Um único byte: pede o STOP já
    while(!(UCB0IFG & UCRXIFG));            // Espera o byte chegar
    uint8_t data = UCB0RXBUF;
    while(UCB0CTL1 & UCTXSTP);
    return data;
}

uint8_t lcdWriteNibble(uint8_t nibble, uint8_t isChar){
    uint8_t byte = (nibble << 4) | LCD_BL | (isChar ? LCD_RS : 0);
    uint8_t nack = 0;
    nack |= i2cSend(LCD_I2C_ADDR, byte);
    nack |= i2cSend(LCD_I2C_ADDR, byte | LCD_EN);
    nack |= i2cSend(LCD_I2C_ADDR, byte);
    return nack;
}

uint8_t lcdWriteByte(uint8_t byte, uint8_t isChar){
    uint8_t nack = lcdWriteNibble(byte >> 4, isChar);
    nack |= lcdWriteNibble(byte & 0x0F, isChar);
    return nack;
}

// Ex. 7: lê 4 bits (D7-D4) do LCD
uint8_t lcdReadNibble(uint8_t isChar){
    uint8_t byte = LCD_DADOS | LCD_BL | LCD_RW | (isChar ? LCD_RS : 0);
    i2cSend(LCD_I2C_ADDR, byte);                    // 1. Prepara RS, R/W = 1, EN = 0
    i2cSend(LCD_I2C_ADDR, byte | LCD_EN);           // 2. EN = 1: LCD põe o dado
    uint8_t lido = i2cRead(LCD_I2C_ADDR);           // 3. Lê no meio do pulso
    i2cSend(LCD_I2C_ADDR, byte);                    // 4. EN = 0
    return (lido >> 4) & 0x0F;
}

uint8_t lcdReadByte(uint8_t isChar){
    uint8_t alto = lcdReadNibble(isChar);           // Primeiro o mais significativo
    uint8_t baixo = lcdReadNibble(isChar);
    return (alto << 4) | baixo;
}

// Ex. 8: retorna o bit BUSY (BIT7 da leitura com RS = 0)
uint8_t lcdBusy(void){
    return (lcdReadByte(0) >> 7) & 0x01;
}

void lcdInit(void){
    __delay_cycles(50000);                  // Espera o LCD ligar (>40 ms)

    // 0x3 três vezes: garante o modo 8 bits partindo de qualquer estado
    // (esperas iguais às do Visto 1, testado na placa)
    lcdWriteNibble(0x3, 0);
    __delay_cycles(20000);
    lcdWriteNibble(0x3, 0);
    __delay_cycles(20000);
    lcdWriteNibble(0x3, 0);
    __delay_cycles(20000);

    lcdWriteNibble(0x2, 0);                 // Modo 4 bits
    __delay_cycles(20000);

    lcdWriteByte(0x28, 0);                  // 4 bits, 2 linhas, fonte 5x8
    __delay_cycles(20000);
    lcdWriteByte(0x0C, 0);                  // Display ligado, cursor desligado
    __delay_cycles(20000);
    lcdWriteByte(0x06, 0);                  // Cursor anda para a direita
    __delay_cycles(2000);
    lcdWriteByte(0x01, 0);
    while(lcdBusy());                       // Em vez de um atraso fixo
}

void main(void){
    WDTCTL = WDTPW | WDTHOLD;

    initI2C_Master();
    lcdInit();

    lcdWriteByte('O', 1);
    lcdWriteByte('l', 1);
    lcdWriteByte('a', 1);

    while(lcdBusy());
    enderecoCursor = lcdReadByte(0) & 0x7F;     // Endereço da DDRAM: 3

    lcdWriteByte(0x80 | 0x00, 0);               // Cursor de volta para o início
    caractereLido = lcdReadByte(1);             // RS = 1: lê a DDRAM -> 'O'
    ocupado = lcdBusy();

    for(;;){
        __no_operation();
    }
}

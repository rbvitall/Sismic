// Módulo 3 - Exercícios 3, 4, 5 e 6: escrita no LCD 16x2 via PCF8574 (I2C)
//   Ex. 3: lcdWriteNibble(nibble, isChar)
//   Ex. 4: lcdWriteByte(byte, isChar)
//   Ex. 5: lcdInit()
//   Ex. 6: lcdWrite(str), com quebra de linha
//
// Pinos do PCF8574 -> LCD:
//   P7-P4: D7-D4 | P3: BL (luz de fundo) | P2: EN | P1: R/W | P0: RS

#include <msp430.h>
#include <stdint.h>

#define LCD_I2C_ADDR 0x27       // Talvez o seu seja 0x3F

#define LCD_RS 0x01             // 0: comando, 1: dado
#define LCD_RW 0x02             // 0: escrita, 1: leitura
#define LCD_EN 0x04             // Pulso de habilitação
#define LCD_BL 0x08             // Luz de fundo

#define LCD_COLUNAS 16

static uint8_t lcdPos = 0;      // Posição do cursor (0..31) para a quebra de linha

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

// Ex. 3: envia 4 bits (D7-D4) com o pulso de EN em 3 transações I2C.
// Cada transação I2C a 100 kHz dura ~0,2 ms, bem mais que os 300 ns
// mínimos do pulso de EN, então não são necessários atrasos extras.
uint8_t lcdWriteNibble(uint8_t nibble, uint8_t isChar){
    uint8_t byte = (nibble << 4) | LCD_BL | (isChar ? LCD_RS : 0);   // R/W = 0
    uint8_t nack = 0;
    nack |= i2cSend(LCD_I2C_ADDR, byte);            // 1. Dados com EN = 0
    nack |= i2cSend(LCD_I2C_ADDR, byte | LCD_EN);   // 2. EN = 1
    nack |= i2cSend(LCD_I2C_ADDR, byte);            // 3. EN = 0 (LCD lê na descida)
    return nack;
}

// Ex. 4: primeiro o nibble mais significativo, depois o menos significativo
uint8_t lcdWriteByte(uint8_t byte, uint8_t isChar){
    uint8_t nack = lcdWriteNibble(byte >> 4, isChar);
    nack |= lcdWriteNibble(byte & 0x0F, isChar);
    return nack;
}

void lcdClear(void){
    lcdWriteByte(0x01, 0);                  // Limpa e volta ao início
    __delay_cycles(2000);                   // Leva 1,53 ms
    lcdPos = 0;
}

// Ex. 5: inicialização em modo 4 bits
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
    lcdClear();
}

// Posiciona o cursor: linha 0 ou 1, coluna 0..15
void lcdCursor(uint8_t linha, uint8_t coluna){
    lcdWriteByte(0x80 | ((linha ? 0x40 : 0x00) + coluna), 0);   // Endereço da DDRAM
    lcdPos = (linha ? LCD_COLUNAS : 0) + coluna;
}

// Ex. 6: escreve a string; ao fim da linha 1 continua na linha 2 e '\n'
// pula para o início da outra linha. Ao fim da linha 2 volta para a linha 1.
void lcdWrite(char *str){
    while(*str){
        char c = *str++;
        if(c == '\n'){
            lcdCursor(lcdPos < LCD_COLUNAS ? 1 : 0, 0);
            continue;
        }
        if(lcdPos == LCD_COLUNAS)
            lcdCursor(1, 0);
        else if(lcdPos >= 2 * LCD_COLUNAS)
            lcdCursor(0, 0);
        lcdWriteByte((uint8_t)c, 1);
        lcdPos++;
    }
}

void main(void){
    WDTCTL = WDTPW | WDTHOLD;

    initI2C_Master();
    __delay_cycles(20000);

    lcdInit();
    lcdWrite("Sistemas Microprocessados 2026");     // Quebra a linha sozinho

    for(;;){
        __no_operation();
    }
}

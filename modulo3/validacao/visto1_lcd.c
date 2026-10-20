// ============================================================================
// Lab. Sistemas Microprocessados 2026.2 - Modulo 3 - VISTO 1
// Escrever mensagem no LCD 16x2 via I2C (PCF8574) - MSP430F5529 LaunchPad
// Exercicios 1, 3, 4, 5 e 6 do Modulo3.pdf
//
// Ligacoes: P3.0 = SDA, P3.1 = SCL, VCC do modulo LCD = 5V da placa, GND = GND
// Se nada aparecer: 1) troque LCD_I2C_ADDR para 0x3F
//                   2) gire o trimpot azul de contraste atras do modulo
// ============================================================================
#include <msp430.h>
#include <stdint.h>

// Endereco I2C do modulo LCD
#define LCD_I2C_ADDR 0x27   // PCF8574T = 0x27 | PCF8574AT = 0x3F

// Pinos do PCF8574
#define LCD_RS 0x01 // 0: Comando, 1: Dados
#define LCD_RW 0x02 // 0: Escrita, 1: Leitura
#define LCD_EN 0x04 // Pulso de habilitacao
#define LCD_BL 0x08 // Luz de fundo
#define LCD_D4 0x10
#define LCD_D5 0x20
#define LCD_D6 0x40
#define LCD_D7 0x80

// Prototipos
void initI2C_Master(void);
uint8_t i2cSend(uint8_t slaveAddr, uint8_t data);
uint8_t lcdWriteNibble(uint8_t nibble, uint8_t isChar);
uint8_t lcdWriteByte(uint8_t byte, uint8_t isChar);
void lcdInit(void);
void lcdWrite(char* str);
void lcdSetCursor(uint8_t linha, uint8_t coluna);

void main(void){
    WDTCTL = WDTPW | WDTHOLD;

    // UCB0: P3.0 = SDA, P3.1 = SCL
    P3DIR &= ~(BIT0 | BIT1);      // DIR = 0 (pede o roteiro)
    P3REN |=  (BIT0 | BIT1);      // Habilita resistores internos...
    P3OUT |=  (BIT0 | BIT1);      // ...como pull-up
    P3SEL |=  (BIT0 | BIT1);      // Funcao dedicada I2C

    initI2C_Master();

    // LCD precisa de >40 ms apos energizar antes da inicializacao
    __delay_cycles(50000);

    lcdInit();

    // Teste do Ex. 6: passa de 16 caracteres -> quebra automatica de linha
    lcdWrite("Teste");
    // Exemplos alternativos:
    // lcdWrite("Lab Microproc.\nModulo 3 - OK");   // '\n' forca a linha 2
    // lcdWrite("Texto bem longo que quebra linha");

    for(;;){
        __no_operation();
    }
}

// ----------------------------------------------------------------------------
// I2C mestre, SMCLK (~1,05 MHz) / 10 = ~100 kHz
// ----------------------------------------------------------------------------
void initI2C_Master(void) {
    UCB0CTL1 |= UCSWRST;                    // Reset para configurar
    UCB0CTL0 = UCMST | UCMODE_3 | UCSYNC;   // Mestre, I2C, sincrono
    UCB0CTL1 = UCSSEL_2 | UCSWRST;          // SMCLK, mantem reset
    UCB0BR0 = 10;
    UCB0BR1 = 0;
    UCB0CTL1 &= ~UCSWRST;                   // Libera o modulo
}

// ----------------------------------------------------------------------------
// Exercicio 1: envia 1 byte ao escravo. Retorna 0 = ACK, 1 = NACK
// ----------------------------------------------------------------------------
uint8_t i2cSend(uint8_t slaveAddr, uint8_t data) {
    uint8_t nack;

    UCB0IFG &= ~UCNACKIFG;                 // Limpa NACK antigo

    UCB0I2CSA = slaveAddr;                 // 1. Endereco do escravo
    UCB0CTL1 |= UCTR | UCTXSTT;            // 2. Transmissor + START

    while (!(UCB0IFG & UCTXIFG));          // 3. Espera TXBUF vazio
    UCB0TXBUF = data;                      //    e escreve o dado

    while (UCB0CTL1 & UCTXSTT);            // 4. Espera ciclo de ACK do endereco

    nack = (UCB0IFG & UCNACKIFG) ? 1 : 0;

    if (!nack) {                           // 5. Escravo respondeu
        while (!(UCB0IFG & UCTXIFG));      //    espera o dado ser transmitido
    }

    UCB0CTL1 |= UCTXSTP;                   // 6. STOP
    while (UCB0CTL1 & UCTXSTP);            // 7. Espera o STOP sair

    return nack;                           // 8. 0 = ACK, 1 = NACK
}

// ----------------------------------------------------------------------------
// Exercicio 3: escreve um nibble (modo 4 bits) com pulso em EN
// Cada i2cSend leva ~200 us a 100 kHz, entao TEN >= 300 ns e TC >= 500 ns
// ficam garantidos sem atrasos extras.
// ----------------------------------------------------------------------------
uint8_t lcdWriteNibble(uint8_t nibble, uint8_t isChar){
    uint8_t err = 0;
    uint8_t dado = ((nibble & 0x0F) << 4)     // D7..D4
                 | LCD_BL                     // backlight sempre ligado
                 | (isChar ? LCD_RS : 0);     // RS = isChar, RW = 0 (escrita)

    err |= i2cSend(LCD_I2C_ADDR, dado);            // 1. Prepara dados, EN = 0
    err |= i2cSend(LCD_I2C_ADDR, dado | LCD_EN);   // 2. EN = 1
    err |= i2cSend(LCD_I2C_ADDR, dado);            // 3. EN = 0 (LCD le na descida)

    return err;   // 0 = tudo com ACK
}

// ----------------------------------------------------------------------------
// Exercicio 4: escreve um byte = nibble alto e depois nibble baixo
// ----------------------------------------------------------------------------
uint8_t lcdWriteByte(uint8_t byte, uint8_t isChar){
    uint8_t err = 0;
    err |= lcdWriteNibble(byte >> 4, isChar);     // MSNibble primeiro
    err |= lcdWriteNibble(byte & 0x0F, isChar);   // LSNibble
    return err;
}

// ----------------------------------------------------------------------------
// Exercicio 5: inicializacao
// ----------------------------------------------------------------------------
void lcdInit(void){
    // 0x3 tres vezes -> garante modo 8 bits partindo de qualquer estado
    uint8_t nibble = 3;
    lcdWriteNibble(nibble, 0);
    __delay_cycles(20000);
    lcdWriteNibble(nibble, 0);
    __delay_cycles(20000);
    lcdWriteNibble(nibble, 0);
    __delay_cycles(20000);

    // Quarta vez: 0x2 -> modo 4 bits
    nibble = 2;
    lcdWriteNibble(nibble, 0);
    __delay_cycles(20000);

    // Configura interface: 4 bits (M=0), 2 linhas (L=1), fonte 5x8 (F=0)
    // (sem este comando o LCD fica em 1 linha e a linha 2 nao aparece)
    lcdWriteByte(0x28, 0);
    __delay_cycles(20000);

    // Display ligado, cursor ligado e piscando
    lcdWriteByte(0x0F, 0);
    __delay_cycles(20000);

    // Limpa e volta o cursor ao inicio (precisa de 1,53 ms)
    lcdWriteByte(0x01, 0);
    __delay_cycles(20000);

    // Entry mode: cursor incrementa, sem deslocar o display
    lcdWriteByte(0x06, 0);
    __delay_cycles(2000);
}

// ----------------------------------------------------------------------------
// Posiciona o cursor: linha 0 -> DDRAM 0x00, linha 1 -> DDRAM 0x40
// Comando "seta endereco da DDRAM" = 1 A6..A0
// ----------------------------------------------------------------------------
void lcdSetCursor(uint8_t linha, uint8_t coluna){
    uint8_t addr = (linha ? 0x40 : 0x00) + (coluna & 0x0F);
    lcdWriteByte(0x80 | addr, 0);
}

// ----------------------------------------------------------------------------
// Exercicio 6: escreve string (termina em 0x00) e gerencia quebra de linha
// - Ao completar 16 caracteres, pula para a outra linha
// - '\n' tambem pula de linha
// Considera que o cursor esta no inicio da linha 1 (apos lcdInit/clear)
// ----------------------------------------------------------------------------
void lcdWrite(char* str){
    uint8_t linha = 0;
    uint8_t coluna = 0;

    while (*str != '\0') {
        char c = *str++;

        if (c == '\n' || coluna >= 16) {
            linha ^= 1;                // alterna linha 1 <-> linha 2
            coluna = 0;
            lcdSetCursor(linha, 0);
            if (c == '\n') continue;   // '\n' nao e impresso
        }

        lcdWriteByte((uint8_t)c, 1);   // RS = 1 -> caractere
        coluna++;
    }
}

// Módulo 3 - Visto 1: escrever uma mensagem no LCD conectado ao MSP430
// Esqueleto do roteiro com as funções dos exercícios 1, 3, 4 e 6 implementadas.
// Endereço I2C do módulo LCD: 0x27 ou 0x3F.

#include <msp430.h>

// Esta biblioteca permite usar o tipo uint8_t
#include <stdint.h>

// Endereço I2C do módulo LCD
#define LCD_I2C_ADDR 0x27 // talvez o seu seja 0x3F

// Definição dos pinos no PCF8574
// P7-P4: D7-D4 (dados)
// P3: BL (backlight)
// P2: EN (enable)
// P1: RW (read/write)
// P0: RS (register select)

#define LCD_RS      0x01 // 0: Comando, 1: Dados
#define LCD_RW      0x02 // 0: Escrita, 1: Leitura
#define LCD_EN      0x04 // Pulso de habilitação
#define LCD_BL      0x08 // Controle da luz de fundo
#define LCD_D4      0x10 // Bit de dados 4
#define LCD_D5      0x20 // Bit de dados 5
#define LCD_D6      0x40 // Bit de dados 6
#define LCD_D7      0x80 // Bit de dados 7

#define LCD_COLUNAS 16

// Protótipos das funções
void initI2C_Master(void);
uint8_t i2cSend(uint8_t slaveAddr, uint8_t data);
uint8_t lcdWriteNibble (uint8_t nibble, uint8_t isChar);
uint8_t lcdWriteByte (uint8_t byte, uint8_t isChar);
void lcdInit(void);
void lcdWrite(char* str);

// Posição atual do cursor (0..31), usada para a quebra de linha
uint8_t lcdPos = 0;

void main(void){
    WDTCTL = WDTPW | WDTHOLD;

    // Configura os pinos GPIO para I2C
    // UCB0: P3.0 = SDA, P3.1 = SCL
    P3SEL |= BIT0 | BIT1;                   // Atribui os pinos ao módulo I2C UCB0
    P3REN |= BIT0 | BIT1;                   // Pull-ups internos (o módulo do LCD
    P3OUT |= BIT0 | BIT1;                   // costuma ter os seus também)

    // Inicializa o módulo I2C
    initI2C_Master();

    // Esperar um pouco antes de realizar outra ação
    __delay_cycles(20000);

    // Inicializar LCD
    lcdInit();

    // Enviar string para ser escrita
    lcdWrite("Teste");

    // Loop infinito
    for(;;){

        __no_operation();
    }
}

void initI2C_Master(void) {

    UCB0CTL1 |= UCSWRST;                    // Reseta para iniciar a configuração
    UCB0CTL0 = UCMST | UCMODE_3 | UCSYNC;   // Mestre, I2C (MODE = 3), síncrono
    UCB0CTL1 = UCSSEL_2 | UCSWRST;          // Usa SMCLK (1 MHz), mantém reset
    UCB0BR0 = 10;                           // SMCLK/10 = 100 kHz
    UCB0BR1 = 0;
    UCB0CTL1 &= ~UCSWRST;                   // Limpa reset para terminar a configuração
}

// Exercício 1: retorna 0 se houve ACK e 1 se houve NACK
uint8_t i2cSend(uint8_t slaveAddr, uint8_t data) {
    UCB0I2CSA = slaveAddr;                  // 1. Endereço do escravo
    UCB0CTL1 |= UCTR | UCTXSTT;             // 2. Transmissor + START

    while(!(UCB0IFG & UCTXIFG));            // 3. Buffer vazio: escreve o dado
    UCB0TXBUF = data;                       //    para destravar o ciclo de ACK

    while(UCB0CTL1 & UCTXSTT);              // 4. Espera o ciclo de acknowledge

    uint8_t nack = (UCB0IFG & UCNACKIFG) ? 1 : 0;
    if(!nack){                              // 5. Escravo respondeu:
        while(!(UCB0IFG & UCTXIFG));        //    espera o dado ser transmitido
    }

    UCB0CTL1 |= UCTXSTP;                    // 6. STOP
    while(UCB0CTL1 & UCTXSTP);              // 7. Espera o STOP ser enviado

    UCB0IFG &= ~UCNACKIFG;
    return nack;                            // 8. ACK (0) ou NACK (1)
}

// Exercício 3: RS = isChar, D7-D4 = nibble, R/W = 0, pulso em EN
uint8_t lcdWriteNibble (uint8_t nibble, uint8_t isChar){
    uint8_t byte = (nibble << 4) | LCD_BL | (isChar ? LCD_RS : 0);
    uint8_t nack = 0;

    nack |= i2cSend(LCD_I2C_ADDR, byte);            // 1. Dados com EN = 0
    nack |= i2cSend(LCD_I2C_ADDR, byte | LCD_EN);   // 2. EN = 1
    nack |= i2cSend(LCD_I2C_ADDR, byte);            // 3. EN = 0

    return nack;
}

// Exercício 4: primeiro os 4 bits mais significativos
uint8_t lcdWriteByte (uint8_t byte, uint8_t isChar){
    uint8_t nack = lcdWriteNibble(byte >> 4, isChar);
    nack |= lcdWriteNibble(byte & 0x0F, isChar);

    return nack;
}

void lcdInit (void){
    // Enviar esse nibble três vezes para garantir que chegamos no modo de 8 bits
    uint8_t nibble = 3;

    lcdWriteNibble(nibble, 0);
    __delay_cycles(20000);
    lcdWriteNibble(nibble, 0);
    __delay_cycles(20000);
    lcdWriteNibble(nibble, 0);
    __delay_cycles(20000);

    // Na quarta vez, enviar 2 para colocar no modo 4 bits
    nibble = 2;
    lcdWriteNibble(nibble, 0);
    __delay_cycles(20000);

    // Configurar a interface: 4 bits, 2 linhas, fonte 5x8 (sem isso a
    // segunda linha não aparece)
    uint8_t byte = 0x28;
    lcdWriteByte(byte, 0);
    __delay_cycles(20000);

    // Configurar display (display ligado, cursor piscando)
    byte = 0x0F;
    lcdWriteByte(byte, 0);
    __delay_cycles(20000);

    // Apagar tudo e levar o cursor para o início
    byte = 0x01;
    lcdWriteByte(byte, 0);
    __delay_cycles(20000);
    lcdPos = 0;
}

// Exercício 6: escreve a string e gerencia a quebra de linha.
// Ao chegar na coluna 16 continua na linha 2; '\n' pula de linha;
// ao fim da linha 2 volta para o início da linha 1.
void lcdWrite(char* str){
    while(*str != 0){
        char c = *str++;

        if(c == '\n'){
            lcdPos = (lcdPos < LCD_COLUNAS) ? LCD_COLUNAS : 0;
            lcdWriteByte(lcdPos ? 0xC0 : 0x80, 0);  // Início da linha 2 ou 1
            continue;
        }

        if(lcdPos == LCD_COLUNAS){
            lcdWriteByte(0xC0, 0);                  // DDRAM 0x40: linha 2
        }
        else if(lcdPos >= 2 * LCD_COLUNAS){
            lcdWriteByte(0x80, 0);                  // DDRAM 0x00: linha 1
            lcdPos = 0;
        }

        lcdWriteByte((uint8_t)c, 1);
        lcdPos++;
    }
}

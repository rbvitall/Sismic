// Módulo 3 - Problema Final - Parte 1: LCD controlado pelas chaves
// O LCD sempre mostra uma das 5 frases da lista.
//   S1 acionada -> frase anterior (da 1 vai para a 5)
//   S2 acionada -> próxima frase  (da 5 vai para a 1)
// Só o acionamento (aberta -> fechada) troca a frase; segurar a chave não
// fica trocando. Rebotes eliminados com espera após cada mudança de estado.
//
// LCD 16x2 com PCF8574 no I2C (USCI_B0: P3.0 = SDA, P3.1 = SCL).

#include <msp430.h>
#include <stdint.h>

#define ABERTO  1
#define FECHADO 0

#define S1 BIT1     // P2.1
#define S2 BIT1     // P1.1

#define LCD_I2C_ADDR 0x27       // Talvez o seu seja 0x3F
#define LCD_RS 0x01
#define LCD_RW 0x02
#define LCD_EN 0x04
#define LCD_BL 0x08
#define LCD_COLUNAS 16

#define N_FRASES 5

// Cada frase tem até 32 caracteres; '\n' separa as duas linhas
const char *frases[N_FRASES] = {
    "Frase 1:\nOla, mundo!",
    "Frase 2:\nSisMic 2026",
    "Frase 3:\nMSP430F5529",
    "Frase 4:\nI2C + LCD 16x2",
    "Frase 5:\nBoa prova!"
};

static uint8_t lcdPos = 0;

// ------------------------------------------------------------------ I2C
void initI2C_Master(void){
    UCB0CTL1 |= UCSWRST;
    UCB0CTL0 = UCMST | UCMODE_3 | UCSYNC;
    UCB0CTL1 = UCSSEL_2 | UCSWRST;          // SMCLK
    UCB0BR0 = 10;                           // ~100 kHz
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

// ------------------------------------------------------------------ LCD
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

void lcdClear(void){
    lcdWriteByte(0x01, 0);
    __delay_cycles(2000);                   // 1,53 ms
    lcdPos = 0;
}

void lcdInit(void){
    __delay_cycles(50000);
    lcdWriteNibble(0x3, 0);
    __delay_cycles(5000);
    lcdWriteNibble(0x3, 0);
    __delay_cycles(200);
    lcdWriteNibble(0x3, 0);
    __delay_cycles(200);
    lcdWriteNibble(0x2, 0);                 // Modo 4 bits
    lcdWriteByte(0x28, 0);                  // 2 linhas, 5x8
    lcdWriteByte(0x0C, 0);                  // Display ligado, sem cursor
    lcdWriteByte(0x06, 0);                  // Incrementa o cursor
    lcdClear();
}

void lcdWrite(const char *str){
    while(*str){
        char c = *str++;
        if(c == '\n'){
            lcdPos = (lcdPos < LCD_COLUNAS) ? LCD_COLUNAS : 0;
            lcdWriteByte(lcdPos ? 0xC0 : 0x80, 0);
            continue;
        }
        if(lcdPos == LCD_COLUNAS)
            lcdWriteByte(0xC0, 0);
        else if(lcdPos >= 2 * LCD_COLUNAS){
            lcdWriteByte(0x80, 0);
            lcdPos = 0;
        }
        lcdWriteByte((uint8_t)c, 1);
        lcdPos++;
    }
}

void mostrarFrase(uint8_t i){
    lcdClear();
    lcdWrite(frases[i]);
}

// ------------------------------------------------------------------ main
void main(void){
    WDTCTL = WDTPW | WDTHOLD;

    P2DIR &= ~S1;  P2REN |= S1;  P2OUT |= S1;
    P1DIR &= ~S2;  P1REN |= S2;  P1OUT |= S2;

    initI2C_Master();
    lcdInit();

    uint8_t atual = 0;                      // Frase 1
    mostrarFrase(atual);

    int X1, X2;
    int X1anterior = ABERTO;
    int X2anterior = ABERTO;

    while(1){
        X1 = (P2IN & S1) ? ABERTO : FECHADO;
        X2 = (P1IN & S2) ? ABERTO : FECHADO;

        if(X1 == FECHADO && X1anterior == ABERTO){      // S1: anterior
            atual = (atual == 0) ? N_FRASES - 1 : atual - 1;
            mostrarFrase(atual);
        }
        else if(X2 == FECHADO && X2anterior == ABERTO){ // S2: próxima
            atual = (atual + 1) % N_FRASES;
            mostrarFrase(atual);
        }

        if(X1 != X1anterior || X2 != X2anterior){
            __delay_cycles(20000);                      // Debounce
        }

        X1anterior = X1;
        X2anterior = X2;
    }
}

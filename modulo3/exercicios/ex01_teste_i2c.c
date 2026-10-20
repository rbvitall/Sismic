// Módulo 3 - Exercício 1: Teste da interface I2C
// Pisca a retroiluminação do LCD (módulo PCF8574) em ~1 Hz.
// USCI_B0 como mestre I2C: P3.0 = SDA, P3.1 = SCL, 100 kHz.
// Byte 0x08 acende a luz de fundo, 0x00 apaga.
// Endereço do PCF8574: 0x27 (PCF8574T) ou 0x3F (PCF8574AT).

#include <msp430.h>
#include <stdint.h>

#define LCD_I2C_ADDR 0x27       // Troque para 0x3F se o seu for o PCF8574AT
#define LCD_BL       0x08

void initI2C_Master(void){
    UCB0CTL1 |= UCSWRST;                    // (1) Reseta a interface
    UCB0CTL0 = UCMST | UCMODE_3 | UCSYNC;   // (2) Mestre, I2C, síncrono
    UCB0CTL1 = UCSSEL_2 | UCSWRST;          //     SMCLK, mantém o reset
    UCB0BR0 = 10;                           //     SMCLK/10 ~ 100 kHz
    UCB0BR1 = 0;

    P3SEL |= BIT0 | BIT1;                   // (3) Pinos na função I2C
    P3DIR &= ~(BIT0 | BIT1);
    P3REN |= BIT0 | BIT1;                   //     Pull-ups internos
    P3OUT |= BIT0 | BIT1;

    UCB0CTL1 &= ~UCSWRST;                   // (4) Libera a interface
}

// Envia um byte ao escravo. Retorna 0 se houve ACK e 1 se houve NACK.
uint8_t i2cSend(uint8_t addr, uint8_t data){
    UCB0I2CSA = addr;                       // 1. Endereço do escravo
    UCB0CTL1 |= UCTR | UCTXSTT;             // 2. Transmissor + START
    while(!(UCB0IFG & UCTXIFG));            // 3. Buffer livre: escreve o dado
    UCB0TXBUF = data;                       //    (destrava o ciclo de ACK)
    while(UCB0CTL1 & UCTXSTT);              // 4. Espera o ciclo de ACK do endereço

    uint8_t nack = (UCB0IFG & UCNACKIFG) ? 1 : 0;
    if(!nack)                               // 5. Escravo respondeu:
        while(!(UCB0IFG & UCTXIFG));        //    espera o dado ser transmitido

    UCB0CTL1 |= UCTXSTP;                    // 6. STOP
    while(UCB0CTL1 & UCTXSTP);              // 7. Espera o STOP ser enviado
    UCB0IFG &= ~UCNACKIFG;
    return nack;                            // 8. 0 = ACK, 1 = NACK
}

void main(void){
    WDTCTL = WDTPW | WDTHOLD;

    initI2C_Master();

    for(;;){
        i2cSend(LCD_I2C_ADDR, LCD_BL);      // Acende
        __delay_cycles(500000);             // ~0,5 s
        i2cSend(LCD_I2C_ADDR, 0x00);        // Apaga
        __delay_cycles(500000);
    }
}

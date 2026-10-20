// Módulo 3 - Problema Final - Parte 2: LCD controlado pelo joystick
// Mesmo comportamento da Parte 1, mas a troca de frase vem de um eixo do
// joystick (eixo X em P6.0/A0), amostrado pelo ADC a cada 100 ms:
//   leitura < L1        -> joystick para um lado   -> frase anterior
//   leitura > L2        -> joystick para o outro   -> próxima frase
//   L1 <= leitura <= L2 -> posição neutra          -> mantém a frase
// A frase muda assim que o joystick sai da região neutra (como o acionamento
// das chaves na Parte 1). Se ele continuar inclinado, depois de 1 s a frase
// passa a mudar a cada 0,5 s, para dar tempo de ler cada uma.
// A faixa neutra larga (L1 < L2) evita que o controle fique sensível demais.
//
// Ligações:
//   LCD (PCF8574): SDA = P3.0, SCL = P3.1
//   Joystick: pino rotulado "+5V" no 3V3 da placa (NÃO use o 5V), GND no GND,
//             VRx -> P6.0

#include <msp430.h>
#include <stdint.h>

#define LCD_I2C_ADDR 0x27       // Talvez o seu seja 0x3F
#define LCD_RS 0x01
#define LCD_RW 0x02
#define LCD_EN 0x04
#define LCD_BL 0x08
#define LCD_COLUNAS 16

#define N_FRASES 5

// Limiares da posição neutra (ADC de 12 bits: 0..4095, centro ~2048)
#define L1 1300
#define L2 2800

// Repetição com o joystick segurado (em amostras de 100 ms)
#define ESPERA_REPETICAO   10   // 1 s
#define PERIODO_REPETICAO  5    // 0,5 s

typedef enum { NEUTRO, ESQUERDA, DIREITA } Posicao;

const char *frases[N_FRASES] = {
    "Frase 1:\nOla, mundo!",
    "Frase 2:\nSisMic 2026",
    "Frase 3:\nMSP430F5529",
    "Frase 4:\nI2C + LCD 16x2",
    "Frase 5:\nBoa prova!"
};

static uint8_t lcdPos = 0;
volatile uint16_t leitura;
volatile uint8_t novaLeitura = 0;

// ------------------------------------------------------------------ I2C
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
    __delay_cycles(20000);
    lcdPos = 0;
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

// ------------------------------------------------------------------ ADC
void initADC(void){
    P6SEL |= BIT0;                                  // A0
    ADC12CTL0 &= ~ADC12ENC;
    ADC12CTL0 = ADC12SHT0_0 | ADC12ON;              // 4 ciclos (> 1,77 us)
    ADC12CTL1 = ADC12SHP | ADC12SHS_0 | ADC12SSEL_3 | ADC12CONSEQ_0;
    ADC12CTL2 = ADC12RES_2;                         // 12 bits
    ADC12MCTL0 = ADC12INCH_0;
    ADC12IE = ADC12IE0;
    ADC12CTL0 |= ADC12ENC;
}

Posicao classificar(uint16_t valor){
    if(valor < L1) return ESQUERDA;
    if(valor > L2) return DIREITA;
    return NEUTRO;
}

// ------------------------------------------------------------------ main
void main(void){
    WDTCTL = WDTPW | WDTHOLD;

    initI2C_Master();
    lcdInit();
    initADC();

    // Timer A0: amostra o joystick a cada 100 ms
    TA0CCR0 = 3277 - 1;                             // 100 ms com ACLK
    TA0CCTL0 = CCIE;
    TA0CTL = TASSEL__ACLK | MC__UP | TACLR;

    uint8_t atual = 0;
    mostrarFrase(atual);
    Posicao anterior = NEUTRO;
    uint16_t amostrasNoLado = 0;

    __enable_interrupt();

    for(;;){
        // Dorme até uma nova leitura (testa a flag com interrupções desligadas)
        __disable_interrupt();
        if(!novaLeitura)
            __bis_SR_register(LPM0_bits | GIE);
        else
            __enable_interrupt();

        if(novaLeitura){
            novaLeitura = 0;
            Posicao agora = classificar(leitura);
            uint8_t mudar = 0;

            if(agora != anterior){                  // Acabou de entrar na região
                amostrasNoLado = 0;
                mudar = (agora != NEUTRO);
                anterior = agora;
            }
            else if(agora != NEUTRO){               // Continua inclinado
                amostrasNoLado++;
                if(amostrasNoLado >= ESPERA_REPETICAO &&
                   (amostrasNoLado - ESPERA_REPETICAO) % PERIODO_REPETICAO == 0)
                    mudar = 1;
            }

            if(mudar){
                if(agora == ESQUERDA)               // Um lado: anterior
                    atual = (atual == 0) ? N_FRASES - 1 : atual - 1;
                else                                // Outro lado: próxima
                    atual = (atual + 1) % N_FRASES;
                mostrarFrase(atual);
            }
        }
    }
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void TA0_CCR0_ISR(void){
    ADC12CTL0 |= ADC12SC;                           // Nova amostra
}

#pragma vector = ADC12_VECTOR
__interrupt void ADC12_ISR(void){
    switch(ADC12IV){
        case ADC12IV_ADC12IFG0:
            leitura = ADC12MEM0;
            novaLeitura = 1;
            ADC12CTL0 &= ~ADC12SC;
            __low_power_mode_off_on_exit();         // Acorda o main
            break;
        default:
            break;
    }
}

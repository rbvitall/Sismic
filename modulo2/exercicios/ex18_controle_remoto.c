// Módulo 2 - Exercício 18: Controle remoto (protocolo NEC, receptor VS1838B)
// Saída do receptor -> P2.0 (TA1.1, CCI1A). O receptor inverte o sinal: cada
// bit começa com um pulso em 0 de 562,5 us. Basta medir o tempo entre bordas
// de DESCIDA consecutivas:
//   início    = 9 ms + 4,5 ms    = 13,5 ms
//   repetição = 9 ms + 2,25 ms   = 11,25 ms
//   bit 0     = 562,5 + 562,5 us = 1,125 ms
//   bit 1     = 562,5 + 1687,5   = 2,25 ms
// Timer A1: SMCLK/8 (~7,63 us por tique), captura na borda de descida.
//
// Os 32 bits são acumulados com o primeiro bit recebido na posição mais
// significativa, que é o formato dos códigos da lista do roteiro [1]
// (ex.: VOL+ = 0x00FFA857).
//
// Uso: VOL+ / VOL- ajustam o brilho do LED vermelho, CH+ / CH- o do verde,
// PLAY deixa os dois em 50% e 0 apaga os dois. Manter o botão pressionado
// (código de repetição) repete o último comando.
// O brilho é um PWM por software no Timer A0 (como no exercício 15).

#include <msp430.h>
#include <stdint.h>

#define LED1 BIT0   // P1.0 vermelho
#define LED2 BIT7   // P4.7 verde
#define IR   BIT0   // P2.0 (TA1.1)

// Códigos do controle (gist do roteiro)
#define TECLA_CH_MENOS  0x00FFA25DUL
#define TECLA_CH_MAIS   0x00FFE21DUL
#define TECLA_VOL_MENOS 0x00FFE01FUL
#define TECLA_VOL_MAIS  0x00FFA857UL
#define TECLA_PLAY      0x00FFC23DUL
#define TECLA_0         0x00FF6897UL

// Janelas de tempo em tiques de SMCLK/8 (1 tique ~ 7,63 us)
#define DENTRO(x, min, max) ((x) >= (min) && (x) <= (max))
#define T_INICIO_MIN  1600      // 13,5 ms ~ 1769
#define T_INICIO_MAX  1950
#define T_REPET_MIN   1350      // 11,25 ms ~ 1475
#define T_REPET_MAX   1599
#define T_BIT0_MIN    100       // 1,125 ms ~ 147
#define T_BIT0_MAX    200
#define T_BIT1_MIN    230       // 2,25 ms ~ 295
#define T_BIT1_MAX    360

#define PERIODO_PWM 100
#define PASSO_PWM   10

volatile uint32_t codigo = 0;           // Último código completo recebido
volatile uint8_t  novoCodigo = 0;
volatile uint8_t  repeticao = 0;

volatile uint16_t brilhoVermelho = PERIODO_PWM / 2;
volatile uint16_t brilhoVerde = PERIODO_PWM / 2;

void atualizarBrilho(void){
    TA0CCR1 = brilhoVermelho;           // CCRn > CCR0 -> LED sempre aceso
    TA0CCR2 = brilhoVerde;
}

void ajustar(volatile uint16_t *brilho, int16_t delta){
    int16_t novo = (int16_t)*brilho + delta;
    if(novo < 0) novo = 0;
    if(novo > PERIODO_PWM) novo = PERIODO_PWM;
    *brilho = novo;
}

void main(void){
    WDTCTL = WDTPW | WDTHOLD;

    P1DIR |= LED1;  P1OUT &= ~LED1;
    P4DIR |= LED2;  P4OUT &= ~LED2;

    P2DIR &= ~IR;
    P2SEL |= IR;

    // Timer A0: PWM por software dos LEDs (ACLK, ~328 Hz)
    TA0CCR0 = PERIODO_PWM - 1;
    atualizarBrilho();
    TA0CCTL1 = CCIE;
    TA0CCTL2 = CCIE;
    TA0CTL = TASSEL__ACLK | MC__UP | TACLR | TAIE;

    // Timer A1: captura das bordas de descida do receptor IR
    TA1CCTL1 = CM_2 | CCIS_0 | SCS | CAP | CCIE;
    TA1CTL = TASSEL__SMCLK | ID__8 | MC__CONTINUOUS | TACLR;

    __enable_interrupt();

    for(;;){
        // Dorme até chegar um comando (testa as flags com interrupções
        // desligadas para não dormir com um comando já pendente)
        __disable_interrupt();
        if(!novoCodigo && !repeticao)
            __bis_SR_register(LPM0_bits | GIE);
        else
            __enable_interrupt();

        if(novoCodigo){
            novoCodigo = 0;
            repeticao = 0;
            switch(codigo){
                case TECLA_VOL_MAIS:  ajustar(&brilhoVermelho, +PASSO_PWM); break;
                case TECLA_VOL_MENOS: ajustar(&brilhoVermelho, -PASSO_PWM); break;
                case TECLA_CH_MAIS:   ajustar(&brilhoVerde, +PASSO_PWM);    break;
                case TECLA_CH_MENOS:  ajustar(&brilhoVerde, -PASSO_PWM);    break;
                case TECLA_PLAY:
                    brilhoVermelho = brilhoVerde = PERIODO_PWM / 2;
                    break;
                case TECLA_0:
                    brilhoVermelho = brilhoVerde = 0;
                    break;
                default:
                    break;
            }
            atualizarBrilho();
        }
        else if(repeticao){             // Botão mantido: repete só os ajustes
            repeticao = 0;
            switch(codigo){
                case TECLA_VOL_MAIS:  ajustar(&brilhoVermelho, +PASSO_PWM); break;
                case TECLA_VOL_MENOS: ajustar(&brilhoVermelho, -PASSO_PWM); break;
                case TECLA_CH_MAIS:   ajustar(&brilhoVerde, +PASSO_PWM);    break;
                case TECLA_CH_MENOS:  ajustar(&brilhoVerde, -PASSO_PWM);    break;
                default: break;
            }
            atualizarBrilho();
        }
    }
}

// Recepção IR: decodifica pelo intervalo entre bordas de descida
#pragma vector = TIMER1_A1_VECTOR
__interrupt void TA1_CCRN_ISR(void){
    static uint16_t ultimaBorda = 0;
    static uint32_t dado = 0;
    static uint8_t  nBits = 0;
    static uint8_t  recebendo = 0;

    switch(TA1IV){
        case TA1IV_TACCR1: {
            uint16_t agora = TA1CCR1;
            uint16_t intervalo = agora - ultimaBorda;
            ultimaBorda = agora;

            if(DENTRO(intervalo, T_INICIO_MIN, T_INICIO_MAX)){
                recebendo = 1;              // Início de um novo comando
                nBits = 0;
                dado = 0;
            }
            else if(DENTRO(intervalo, T_REPET_MIN, T_REPET_MAX)){
                recebendo = 0;              // Código de repetição
                repeticao = 1;
                __low_power_mode_off_on_exit();
            }
            else if(recebendo && DENTRO(intervalo, T_BIT0_MIN, T_BIT0_MAX)){
                dado <<= 1;                 // Bit 0
                nBits++;
            }
            else if(recebendo && DENTRO(intervalo, T_BIT1_MIN, T_BIT1_MAX)){
                dado = (dado << 1) | 1;     // Bit 1
                nBits++;
            }
            else{
                recebendo = 0;              // Intervalo inválido: descarta
            }

            if(recebendo && nBits == 32){
                recebendo = 0;
                codigo = dado;
                novoCodigo = 1;
                __low_power_mode_off_on_exit();
            }
            break;
        }
        default:
            break;
    }
}

// PWM por software dos LEDs
#pragma vector = TIMER0_A1_VECTOR
__interrupt void TA0_CCRN_ISR(void){
    switch(TA0IV){
        case TA0IV_TACCR1: P1OUT &= ~LED1; break;
        case TA0IV_TACCR2: P4OUT &= ~LED2; break;
        case TA0IV_TAIFG:
            // Com brilho 0 o CCRn coincide com o overflow: mantém apagado
            if(brilhoVermelho) P1OUT |= LED1;
            if(brilhoVerde)    P4OUT |= LED2;
            break;
        default:
            break;
    }
}

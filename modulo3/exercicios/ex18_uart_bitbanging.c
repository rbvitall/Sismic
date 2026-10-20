// Módulo 3 - Exercício 18: exercício 17 com UART por bit-banging
// Mesmos pinos (P3.3 = TX, P3.4 = RX), agora como GPIO, e mesma taxa (1200 bps).
// Mesmo comportamento: S1 envia 0x01/0x0A, S2 envia 0x10/0xA0, e os códigos
// recebidos acendem/apagam os LEDs.
//
// Temporização com o Timer A0 em modo contínuo (SMCLK = 1048576 Hz):
//   1 bit = 1048576 / 1200 ~ 874 tiques (~833 us)
// O próximo instante é sempre calculado somando ao anterior (CCR0 += BIT),
// assim os erros não se acumulam.
// Recepção: como a porta P3 não tem interrupção, o main fica observando RX.
// Ao ver o start bit (descida), espera 1,5 bit para cair no CENTRO do bit 0
// e então amostra cada bit no centro da sua janela (Figura 18).

#include <msp430.h>
#include <stdint.h>

#define ABERTO  1
#define FECHADO 0

#define S1   BIT1   // P2.1
#define S2   BIT1   // P1.1
#define LED1 BIT0   // P1.0 vermelho
#define LED2 BIT7   // P4.7 verde
#define TX   BIT3   // P3.3
#define RX   BIT4   // P3.4

#define BIT_TIQUES 874

// Espera até o contador chegar em TA0CCR0 (que já foi ajustado)
static void esperaMarca(void){
    TA0CCTL0 &= ~CCIFG;
    while(!(TA0CCTL0 & CCIFG));
}

void bbSend(uint8_t b){
    uint8_t i;
    TA0CCR0 = TA0R + BIT_TIQUES;
    P3OUT &= ~TX;                           // Start bit
    esperaMarca();
    for(i = 0; i < 8; i++){                 // 8 bits, LSB primeiro
        if(b & 0x01) P3OUT |= TX; else P3OUT &= ~TX;
        b >>= 1;
        TA0CCR0 += BIT_TIQUES;
        esperaMarca();
    }
    P3OUT |= TX;                            // Stop bit
    TA0CCR0 += BIT_TIQUES;
    esperaMarca();
}

// Chamada logo após detectar a borda de descida do start bit
uint8_t bbReceive(void){
    uint8_t b = 0, i;
    TA0CCR0 = TA0R + BIT_TIQUES + BIT_TIQUES / 2;   // Centro do bit 0
    esperaMarca();
    for(i = 0; i < 8; i++){
        b >>= 1;
        if(P3IN & RX) b |= 0x80;
        TA0CCR0 += BIT_TIQUES;
        esperaMarca();
    }
    // Agora estamos no centro do stop bit: a linha já está em 1
    return b;
}

void tratarCodigo(uint8_t c){
    switch(c){
        case 0x01: P1OUT |= LED1;  break;
        case 0x0A: P1OUT &= ~LED1; break;
        case 0x10: P4OUT |= LED2;  break;
        case 0xA0: P4OUT &= ~LED2; break;
        default: break;
    }
}

void main(void){
    WDTCTL = WDTPW | WDTHOLD;

    P2DIR &= ~S1;  P2REN |= S1;  P2OUT |= S1;
    P1DIR &= ~S2;  P1REN |= S2;  P1OUT |= S2;
    P1DIR |= LED1;  P1OUT &= ~LED1;
    P4DIR |= LED2;  P4OUT &= ~LED2;

    P3DIR |= TX;   P3OUT |= TX;             // Linha ociosa em 1
    P3DIR &= ~RX;  P3REN |= RX;  P3OUT |= RX;   // Pull-up mantém RX em 1 se desconectado

    TA0CTL = TASSEL__SMCLK | MC__CONTINUOUS | TACLR;

    uint8_t ligarVermelho = 1;
    uint8_t ligarVerde = 1;

    int X1, X2;
    int X1anterior = ABERTO;
    int X2anterior = ABERTO;

    while(1){
        if((P3IN & RX) == 0){               // Start bit chegando
            tratarCodigo(bbReceive());
        }

        X1 = (P2IN & S1) ? ABERTO : FECHADO;
        X2 = (P1IN & S2) ? ABERTO : FECHADO;

        if(X1 == FECHADO && X1anterior == ABERTO){
            bbSend(ligarVermelho ? 0x01 : 0x0A);
            ligarVermelho = !ligarVermelho;
        }
        if(X2 == FECHADO && X2anterior == ABERTO){
            bbSend(ligarVerde ? 0x10 : 0xA0);
            ligarVerde = !ligarVerde;
        }

        // Debounce sem __delay_cycles longo, para não perder um start bit:
        // espera 20 ms em fatias de 1 bit, observando a linha RX
        if(X1 != X1anterior || X2 != X2anterior){
            uint8_t k;
            TA0CCR0 = TA0R + BIT_TIQUES;
            for(k = 0; k < 24; k++){
                if((P3IN & RX) == 0){
                    tratarCodigo(bbReceive());
                    TA0CCR0 = TA0R + BIT_TIQUES;
                }
                esperaMarca();
                TA0CCR0 += BIT_TIQUES;
            }
        }

        X1anterior = X1;
        X2anterior = X2;
    }
}

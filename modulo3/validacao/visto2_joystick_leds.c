// Módulo 3 - Visto 2: Exercício 14 do Modulo3.pdf
// Brilho dos dois LEDs controlado pelos dois eixos do joystick.
//   Eixo X (P6.0/A0) -> LED vermelho | Eixo Y (P6.1/A1) -> LED verde
//   Joystick no centro -> os dois LEDs em ~50%.
//
// - PWM de ~100 Hz com 256 passos nos canais 1 e 2 do Timer B0
//   (SMCLK / 8 / 5 = 26214 Hz; 26214 / 256 ~ 102 Hz).
// - ADC12 com 8 bits (mesma resolução do PWM), amostrado a cada 20 ms.
// - A interrupção do ADC copia os resultados para TB0CCR1 e TB0CCR2.
//
// Ligações:
//   Joystick: pino rotulado "+5V" no 3V3 da placa (NÃO use o 5V), GND no GND,
//             VRx -> P6.0, VRy -> P6.1.
//   LED verde: P4.7 já é o LED (remapeado para TB0CCR2A).
//   LED vermelho: retire o jumper JP8 e ligue P4.3 (TB0CCR1A) ao pino de
//   BAIXO do jumper (o mais próximo da borda da placa).

#include <msp430.h>
#include <stdint.h>

void initPWM_TB0(void){
    // Mapeamento das saídas do Timer B0 na porta P4
    PMAPKEYID = 0x02D52;                // Senha para liberar a config do PMAP
    P4MAP3 = PM_TB0CCR1A;               // Conecta P4.3 na saída do canal 1 do timer B
    P4MAP7 = PM_TB0CCR2A;               // Conecta P4.7 na saída do canal 2 do timer B
    PMAPKEYID = 0;
    P4DIR |= BIT3 | BIT7;
    P4SEL |= BIT3 | BIT7;

    TB0CCR0 = 256 - 1;                  // 256 passos
    TB0CCR1 = 128;                      // Começa em 50%
    TB0CCR2 = 128;
    TB0CCTL1 = OUTMOD_7;                // Reset/Set
    TB0CCTL2 = OUTMOD_7;
    TB0EX0 = TBIDEX_4;                  // /5
    TB0CTL = TBSSEL__SMCLK | ID__8 | MC__UP | TBCLR;    // /8 -> ~102 Hz
}

void initADC(void){
    P6SEL |= BIT0 | BIT1;               // Função alternativa de P6.0 e P6.1

    ADC12CTL0 &= ~ADC12ENC;             // Desabilitar para iniciar configuração

    // 4 ciclos de amostragem (> 1,77 us calculado no exercício 9),
    // MSC: depois do gatilho converte os dois canais em sequência, liga o ADC
    ADC12CTL0 = ADC12SHT0_0 | ADC12MSC | ADC12ON;

    // pulse mode (SHP=1), gatilho ADC12SC (SHS=0), SMCLK (SSEL=3),
    // sequência de canais (CONSEQ=1) começando em MEM0
    ADC12CTL1 = ADC12CSTARTADD_0 | ADC12SHP | ADC12SHS_0 | ADC12SSEL_3 | ADC12CONSEQ_1;

    ADC12CTL2 = ADC12RES_0;             // 8 bits: 0..255, igual ao PWM

    ADC12MCTL0 = ADC12INCH_0;           // MEM0 <- A0 (X)
    ADC12MCTL1 = ADC12INCH_1 | ADC12EOS;// MEM1 <- A1 (Y), fim da sequência

    ADC12IE = ADC12IE1;                 // Interrupção quando MEM1 ficar pronto

    ADC12CTL0 |= ADC12ENC;              // Habilitar conversão
}

void main(void){
    WDTCTL = WDTPW | WDTHOLD;

    initPWM_TB0();
    initADC();

    // Timer A0: dispara a leitura do joystick a cada 20 ms (50 Hz)
    TA0CCR0 = 655 - 1;                  // 655 tiques do ACLK ~ 20 ms
    TA0CCTL0 = CCIE;
    TA0CTL = TASSEL__ACLK | MC__UP | TACLR;

    __enable_interrupt();

    for(;;){
        __low_power_mode_0();           // SMCLK continua ligado (PWM e ADC)
    }
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void TA0_CCR0_ISR(void){
    ADC12CTL0 |= ADC12SC;               // Inicia a sequência X, Y
}

#pragma vector = ADC12_VECTOR
__interrupt void ADC12_ISR(void) {
    switch(ADC12IV){
        case ADC12IV_ADC12IFG1:         // X e Y prontos
            TB0CCR1 = ADC12MEM0;        // Brilho do vermelho
            TB0CCR2 = ADC12MEM1;        // Brilho do verde
            ADC12CTL0 &= ~ADC12SC;
            break;
        default:
            break;
    }
}

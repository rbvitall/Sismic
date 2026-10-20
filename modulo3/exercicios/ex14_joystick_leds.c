// Módulo 3 - Exercício 14 (também é o Visto 2): brilho dos LEDs pelo joystick
// Eixo X (P6.0/A0) -> brilho do LED vermelho
// Eixo Y (P6.1/A1) -> brilho do LED verde
// Joystick no centro -> os dois LEDs em ~50%.
//
// - PWM de ~100 Hz com 256 passos nos canais 1 e 2 do Timer B0:
//   SMCLK / 8 / 5 = 26214 Hz; 26214 / 256 ~ 102 Hz
// - ADC de 8 bits (mesma resolução do PWM), amostrando a cada 20 ms.
// - O resultado de cada eixo vai direto para TB0CCR1/TB0CCR2 na interrupção
//   do ADC (alternativa ao DMA permitida no roteiro).
//
// Mapeamento de pinos da porta P4 (PMAP):
//   P4.7 (LED verde) <- TB0CCR2A
//   P4.3             <- TB0CCR1A. Para o LED vermelho, retire o jumper JP8 e
//   ligue P4.3 ao pino de BAIXO do jumper (o mais próximo da borda da placa).
//
// Joystick: pino "+5V" do módulo no 3V3 da placa (NUNCA no 5V).

#include <msp430.h>
#include <stdint.h>

void main(void){
    WDTCTL = WDTPW | WDTHOLD;

    // Mapeamento das saídas do Timer B0 na porta P4
    PMAPKEYID = 0x02D52;                // Senha para liberar o PMAP
    P4MAP3 = PM_TB0CCR1A;               // P4.3 <- canal 1 do Timer B0
    P4MAP7 = PM_TB0CCR2A;               // P4.7 <- canal 2 do Timer B0
    PMAPKEYID = 0;                      // Trava o PMAP de novo
    P4DIR |= BIT3 | BIT7;
    P4SEL |= BIT3 | BIT7;

    // Timer B0: PWM de 256 passos
    TB0CCR0 = 256 - 1;
    TB0CCR1 = 128;
    TB0CCR2 = 128;
    TB0CCTL1 = OUTMOD_7;
    TB0CCTL2 = OUTMOD_7;
    TB0EX0 = TBIDEX_4;                  // Divide por 5
    TB0CTL = TBSSEL__SMCLK | ID__8 | MC__UP | TBCLR;

    // ADC: sequência A0, A1 disparada por ADC12SC a cada 20 ms
    P6SEL |= BIT0 | BIT1;
    ADC12CTL0 &= ~ADC12ENC;
    ADC12CTL0 = ADC12SHT0_0 | ADC12MSC | ADC12ON;   // MSC: converte os 2 canais seguidos
    ADC12CTL1 = ADC12CSTARTADD_0 | ADC12SHP | ADC12SHS_0 | ADC12SSEL_3 | ADC12CONSEQ_1;
    ADC12CTL2 = ADC12RES_0;                         // 8 bits
    ADC12MCTL0 = ADC12INCH_0;                       // X
    ADC12MCTL1 = ADC12INCH_1 | ADC12EOS;            // Y
    ADC12IE = ADC12IE1;
    ADC12CTL0 |= ADC12ENC;

    // Timer A0: base de tempo de 20 ms
    TA0CCR0 = 655 - 1;                              // 20 ms com ACLK
    TA0CCTL0 = CCIE;
    TA0CTL = TASSEL__ACLK | MC__UP | TACLR;

    __enable_interrupt();

    for(;;){
        __low_power_mode_0();
    }
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void TA0_CCR0_ISR(void){
    ADC12CTL0 |= ADC12SC;                           // Inicia a sequência X, Y
}

#pragma vector = ADC12_VECTOR
__interrupt void ADC12_ISR(void){
    switch(ADC12IV){
        case ADC12IV_ADC12IFG1:
            TB0CCR1 = ADC12MEM0;                    // 0..255 -> 0..100% (vermelho)
            TB0CCR2 = ADC12MEM1;                    // 0..255 -> 0..100% (verde)
            ADC12CTL0 &= ~ADC12SC;
            break;
        default:
            break;
    }
}

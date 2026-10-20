// ============================================================================
// Lab. Sistemas Microprocessados 2026.2 - Modulo 3 - Exercicio 14 (igual ao Visto 2)
// Joystick (2 eixos) controla o brilho dos LEDs via PWM - MSP430F5529
//
//  - Eixo X -> LED vermelho | Eixo Y -> LED verde
//  - PWM ~100 Hz com 256 passos (0..255) no Timer B (canais 1 e 2)
//  - ADC12 em 8 bits (mesma resolucao da PWM), amostrando a cada 20 ms
//  - Interrupcao do ADC copia o resultado para TB0CCR1 / TB0CCR2
//  - Joystick no centro (~128) -> LEDs em ~50%
//
// LIGACOES (confirme com o professor):
//   Joystick +5V -> 3V3 da placa  (NUNCA no 5V!)
//   Joystick GND -> GND
//   Joystick VRx -> P6.0 (A0)
//   Joystick VRy -> P6.1 (A1)
//   LED verde (LED2) = P4.7 -> recebe TB0.2 direto pelo port mapping
//   LED vermelho (LED1) = P1.0 -> P1.0 nao tem saida do Timer B. Duas opcoes:
//     (a) LED_VERMELHO_SOFTWARE = 1: o proprio codigo espelha a PWM em P1.0
//         usando interrupcoes do Timer B (nao precisa de fio).
//     (b) LED_VERMELHO_SOFTWARE = 0: TB0.1 sai em P4.3 (como no roteiro);
//         ligue um jumper de P4.3 ate o pino P1.0 do header (P1.0 fica entrada).
//
// Ex. 9 (calculo do tempo de amostragem), pot de 10 kOhm:
//   Rs = Rpot/4 = 2,5 k ; (Rs + Ri)*Ci = 4,3k * 25p = 107,5 ns
//   tcap = 107,5 ns * ln(2^13) + 800 ns = 107,5*9,01 + 800 ~= 1,77 us
//   Com clock de ~1 MHz bastam 2 ciclos -> SHT = 4 ciclos (SHT0_0) ja cobre.
//   Aqui usamos SHT0_2 (16 ciclos) com folga.
// ============================================================================
#include <msp430.h>
#include <stdint.h>

#define LED_VERMELHO_SOFTWARE 1   // ver opcoes (a)/(b) acima

void initPWM_TimerB(void);
void initADC_Joystick(void);
void initTimerA_20ms(void);

void main(void) {
    WDTCTL = WDTPW | WDTHOLD;

    initPWM_TimerB();
    initADC_Joystick();
    initTimerA_20ms();

    __enable_interrupt();

    for(;;){
        __bis_SR_register(LPM0_bits);   // Dorme; SMCLK continua ativo
        __no_operation();
    }
}

// ----------------------------------------------------------------------------
// Timer B0: PWM ~100 Hz, 256 passos
// SMCLK = 1.048.576 Hz / 8 (ID_3) / 5 (TBIDEX_4) = 26.214 Hz
// Periodo = 256 contagens -> 26.214/256 = 102,4 Hz (~100 Hz)
// ----------------------------------------------------------------------------
void initPWM_TimerB(void) {
    // Port mapping: TB0.1 -> P4.3, TB0.2 -> P4.7 (LED verde)
    PMAPKEYID = 0x02D52;            // Senha para liberar o PMAP
    P4MAP3 = PM_TB0CCR1A;
    P4MAP7 = PM_TB0CCR2A;
    PMAPKEYID = 0;                  // Trava o PMAP de novo

    P4DIR |= BIT3 | BIT7;
    P4SEL |= BIT3 | BIT7;           // Funcao dedicada (saida do timer)

    TB0CCR0  = 255;                 // 256 passos (0..255)
    TB0CCR1  = 128;                 // Comeca em 50%
    TB0CCR2  = 128;
    TB0CCTL1 = OUTMOD_7;            // Reset/Set: duty = CCRx/256
    TB0CCTL2 = OUTMOD_7;

#if LED_VERMELHO_SOFTWARE
    P1DIR |= BIT0;
    P1OUT &= ~BIT0;
    TB0CCTL0 |= CCIE;               // Inicio do periodo -> acende
    TB0CCTL1 |= CCIE;               // Fim do duty       -> apaga
#endif

    TB0EX0 = TBIDEX_4;              // Divide por 5
    TB0CTL = TBSSEL_2 | ID_3 | MC_1 | TBCLR;   // SMCLK, /8, modo up
}

// ----------------------------------------------------------------------------
// ADC12: sequencia de 2 canais (A0 = X, A1 = Y), 8 bits
// Disparo por software (ADC12SC) feito pelo Timer A a cada 20 ms.
// MSC = 1 -> um disparo converte a sequencia inteira (A0 e A1).
// ----------------------------------------------------------------------------
void initADC_Joystick(void) {
    P6SEL |= BIT0 | BIT1;                       // P6.0 e P6.1 analogicos

    ADC12CTL0 &= ~ADC12ENC;                     // Desabilita para configurar
    ADC12CTL0 = ADC12SHT0_2 | ADC12MSC | ADC12ON;   // 16 ciclos, multi, liga
    ADC12CTL1 = ADC12SHP                        // Pulse mode (timer interno)
              | ADC12SHS_0                      // Gatilho = ADC12SC
              | ADC12SSEL_3                     // SMCLK
              | ADC12CONSEQ_1;                  // Sequencia de canais, unica
    ADC12CTL2 = ADC12RES_0;                     // 8 bits -> 0..255

    ADC12MCTL0 = ADC12INCH_0;                   // MEM0 <- A0 (eixo X)
    ADC12MCTL1 = ADC12INCH_1 | ADC12EOS;        // MEM1 <- A1 (eixo Y), fim

    ADC12IE = ADC12IE1;                         // Interrompe ao fim (MEM1)
    ADC12CTL0 |= ADC12ENC;                      // Habilita conversao
}

// ----------------------------------------------------------------------------
// Timer A0: interrupcao a cada 20 ms (50 Hz) para disparar o ADC
// SMCLK / 8 = 131.072 Hz -> 20 ms = 2621 contagens
// ----------------------------------------------------------------------------
void initTimerA_20ms(void) {
    TA0CCR0  = 2621 - 1;
    TA0CCTL0 = CCIE;
    TA0CTL   = TASSEL_2 | ID_3 | MC_1 | TACLR;  // SMCLK, /8, modo up
}

// ============================ INTERRUPCOES ==================================

// A cada 20 ms: inicia a conversao da sequencia
#pragma vector = TIMER0_A0_VECTOR
__interrupt void TA0_ISR(void) {
    ADC12CTL0 |= ADC12SC;
}

// Fim da sequencia: move os resultados para a largura de pulso da PWM
#pragma vector = ADC12_VECTOR
__interrupt void ADC12_ISR(void) {
    switch (__even_in_range(ADC12IV, 34)) {
        case ADC12IV_ADC12IFG1:
            TB0CCR1 = ADC12MEM0;    // Eixo X -> LED vermelho
            TB0CCR2 = ADC12MEM1;    // Eixo Y -> LED verde (leitura limpa flags)
            break;
        default:
            break;
    }
}

#if LED_VERMELHO_SOFTWARE
// Inicio do periodo PWM: acende LED vermelho (se duty > 0)
#pragma vector = TIMER0_B0_VECTOR
__interrupt void TB0_CCR0_ISR(void) {
    if (TB0CCR1 > 0) P1OUT |= BIT0;
}

// Contador atingiu CCR1: apaga LED vermelho (se duty < 100%)
#pragma vector = TIMER0_B1_VECTOR
__interrupt void TB0_CCRx_ISR(void) {
    switch (__even_in_range(TB0IV, 14)) {
        case TB0IV_TB0CCR1:
            if (TB0CCR1 < 255) P1OUT &= ~BIT0;
            break;
        default:
            break;
    }
}
#endif

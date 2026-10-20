// Módulo 2 - Exercício 3: Remoção de rebotes
// Mesmo comportamento do exercício 2, agora com debounce feito por uma
// função que gasta tempo decrementando uma variável volatile (o volatile
// impede que o otimizador do compilador remova o laço vazio).

#include <msp430.h>

#define S1   BIT1   // P2.1
#define LED1 BIT0   // P1.0 (vermelho)

void debounce(void){
    volatile unsigned int i = 2000;     // ~20 ms com MCLK ~1 MHz (ajuste se preciso)
    while(i != 0)
        i--;
}

void main(void){
    WDTCTL = WDTPW | WDTHOLD;

    P2DIR &= ~S1;
    P2REN |= S1;
    P2OUT |= S1;

    P1DIR |= LED1;
    P1OUT &= ~LED1;

    for(;;){
        while((P2IN & S1) != 0);    // Espera a chave fechar
        P1OUT ^= LED1;              // Ação
        debounce();                 // Espera os rebotes do fechamento
        while((P2IN & S1) == 0);    // Espera a chave abrir
        debounce();                 // Espera os rebotes da abertura
    }
}

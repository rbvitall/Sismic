/* msp430.h falso para testar a lógica das ISRs no PC (tools/host) */
#include <stdint.h>
#define BIT0 0x01
#define BIT1 0x02
#define BIT2 0x04
#define BIT3 0x08
#define BIT7 0x80
#define WDTPW 0x5A00
#define WDTHOLD 0x80
#define OUTMOD_7 0xE0
#define CCIE 0x10
#define TASSEL__SMCLK 0x200
#define TASSEL__ACLK 0x100
#define MC__UP 0x10
#define TACLR 0x04
extern volatile uint16_t WDTCTL, TA0CCR0, TA0CCTL0, TA0CTL, TA1CCR0, TA1CCR1, TA1CCTL1, TA1CTL;
extern volatile uint8_t P1DIR, P1REN, P1OUT, P1IN, P2DIR, P2REN, P2OUT, P2IN, P2SEL;
#define __interrupt
#define __enable_interrupt()
#define __low_power_mode_0()

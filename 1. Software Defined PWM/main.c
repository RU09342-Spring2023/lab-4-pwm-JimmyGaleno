/*
 * Part1.c
 *
 *  Created on: March 7, 2023
 *      Author: Jimmy Galeno
 *
 *      The task for this portion of the lab is to generate a 1 kHz PWM signal with a duty cycle between 0-100%.
 *      When turned on, LED's should be at a duty cycle of 50%, then it should increase by increments of 10%. This will
 *      be determined by what button is being pressed. For the red LED, button 2.1 will be used, and for the green LED,
 *      button 4.3 will be used.
 *
*/

#include <msp430.h>

unsigned short defaultT = 999;                      // declare and initialize a variable for the default timer period
unsigned short dutyCycleIncrement = 100;            // declare and initialize a variable for the duty cycle increment


void gpioInit();
void timerInit();

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    gpioInit();
    timerInit();

    PM5CTL0 &= ~LOCKLPM5; // disable GPIO power-on default high-impedance mode to active previously configured port settings

    __bis_SR_register(LPM3_bits | GIE); // enter LPM3, enable interrupts
    __no_operation(); // no operation for debugger

}


void gpioInit() {

    P1OUT &= ~BIT0; // reset P1.0 red LED
    P1DIR |= BIT0; // set P1.0 to output

    P6OUT &= ~BIT6; // reset P6.6 green LED
    P6DIR |= BIT6; // set P6.6 to output


    P2DIR &= ~BIT3; // set P2.3 to input
    P2REN |= BIT3; // enable P2.3 resistor
    P2OUT |= BIT3; // set P2.3 resistor to pull-up
    P2IES |= BIT3; // P2.3 High -> Low edge
    P2IE |= BIT3; // P2.3 interrupt enable
    P2IFG &= ~BIT3; // clear P2.3 interrupt flag

    P4DIR &= ~BIT1; // set P4.1 to input
    P4REN |= BIT1; // enable P4.1 resistor
    P4OUT |= BIT1; // set P4.1 resistor to pull-up
    P4IES |= BIT1; // P4.1 High -> Low edge
    P4IE |= BIT1; // P4.1 interrupt mode
    P4IFG &= ~BIT1; // clear P4.1 interrupt flag

}

void timerInit() {
    TB0CCTL1 = TB0CCTL2 |= CCIE; // TBCCR1, TBCCR2, and TBCC2 interrupt enabled
    TB0CCR0 = defaultT;          // set to defaultT which is equal to 999
    TB0CCR1 = TB0CCR2 = (defaultT>>1); // initial duty cycle is default period / 2
    TB0CTL = TBSSEL_1 | MC_1 | TBCLR | TBIE; // ACLK, up mode, clear TBR, enable interrupt
}

// port 2.3 triggered
#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void) {
    P2IFG &= ~BIT3; // clear P2.3 interrupt flag
    if(TB0CCR1 >= defaultT)    //TBOCCR1 greater or equal to 999
        TB0CCR1 = 0;           // reset to 0
    else
        TB0CCR1 += dutyCycleIncrement;     // else increment TBOCCR1 by the duty cycle
}

// port 4.3 triggered
#pragma vector=PORT4_VECTOR
__interrupt void Port_4(void) {
    P4IFG &= ~BIT1; // clear P4.3 interrupt flag
    if(TB0CCR2 >= defaultT)    //TBOCCR2 greater or equal to 999
        TB0CCR2 = 0;           // reset to 0
    else
        TB0CCR2 += dutyCycleIncrement;      // else increment TBOCCR2 by the duty cycle
}

// Timer0_B3 interrupt vector (TBIV) handler
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=TIMER0_B1_VECTOR
__interrupt void Timer0_B1_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER0_B1_VECTOR))) TIMER0_B1_ISR (void)
#else
#error Compiler not supported!
#endif
{
    switch(__even_in_range(TB0IV,TB0IV_TBIFG)) {
    case TB0IV_NONE: // no interrupt
        break;
    case TB0IV_TBCCR1: // CCR1
        P1OUT &= ~BIT0;
        break;
    case TB0IV_TBCCR2: // CCR2
        P6OUT &= ~BIT6;
        break;
    case TB0IV_TBCCR3: // CCR3
        break;
    case TB0IV_TBIFG: // overflow
        P1OUT |= BIT0;
        P6OUT |= BIT6;
        break;
    }
}




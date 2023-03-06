/*
 * SoftwarePWM.c
 *
 *  Created on: Feb 18, 2023
 *      Author: Russell Trafford and Craig Droke
 *
 *
 * This software should create a 1 kHz PWM and both LED's should have a 50% duty cycle. Pressing the button interupts should increase the brightness of the
 * respective one by 10%.
 *
 */

#include <msp430.h>

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;                   // Stop WDT

        //Red LED init
        P1DIR |= BIT0;                          // Set P1.0 to out
        P1OUT &= ~BIT0;                         // Clear P1.0

        //Green LED init
        P6DIR |= BIT6;                          // Set P6.6 to out
        P6OUT &= ~BIT6;                         // Clear P6.6

        //P2.3 button interupt
        P2DIR &= ~BIT3;
        P2OUT |= BIT3;                          // Configure P2.3 as pulled-up
        P2REN |= BIT3;                          // P2.3 pull-up register enable
        P2IES &= ~BIT3;                         // P2.3 Low --> High edge
        P2IE |= BIT3;                           // P2.3 interrupt enabled

        //P4.1 button interupt
        P4OUT &= ~BIT1;
        P4OUT |= BIT1;                          // Configure P4.1 as pulled-up
        P4REN |= BIT1;                          // P4.1 pull-up register enable
        P4IES &= ~BIT1;                         // P4.1 Low --> High edge
        P4IE |= BIT1;                           // P4.1 interrupt enabled

    PM5CTL0 &= ~LOCKLPM5;


    //Timer_A Init
    TB0CTL = TBSSEL__SMCLK | MC__UP | TBIE;              // UP mode (The SMclk)
    TB0CCTL1 |= CCIE;                                    // Enable TB0 CCR1 Interrupt
    TB0CCTL2 |= CCIE;                                    // Enable TB0 CCR1 Interrupt
    TB0CCR0 = 1000;                                      // Set CCR0 to the value to set the period (1 kHz)
    TB0CCR1 = 500;                                       // Set CCR1 to the Duty Cycle (50%)
    TB0CCR2 = 500;                                       // Set CCR2 to the Duty Cycle (50%)


    __bis_SR_register(LPM3_bits | GIE);           // Enter LPM3, enable interrupts
    __no_operation();
}

// Port 4 interrupt service routine
#pragma vector=PORT4_VECTOR
__interrupt void Port_4(void)
{
    P4IFG &= ~BIT1;                                     //Clears in flag
        if(TB0CCR2 >= 999){                             //At max brightness (100%) reset to 0
            TB0CCR2 = 30;                               //Set capture compare to 30
        }
        else{
            TB0CCR2 += 100;                             //10% brightness change
        }
}

// Port 2 interrupt service routine
#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
{
    P2IFG &= ~BIT3;                                      // Clears int flag
    if(TB0CCR1 >= 999){                                  //At max brightness (100%) reset to 0
        TB0CCR1 = 30;                                    //Set capture compare to 30
    }
    else{
        TB0CCR1 += 100;                                  //10% brightness change
    }

}

// Timer0_B3 Interrupt Vector (TBIV) handler
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=TIMER0_B1_VECTOR
__interrupt void TIMER0_B1_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER0_B1_VECTOR))) TIMER0_B1_ISR (void)
#else
#error Compiler not supported!
#endif
{
    switch(__even_in_range(TB0IV,TB0IV_TBIFG))
    {
        case TB0IV_NONE:
            break;                               // No interrupt
        case TB0IV_TBCCR1:
            P1OUT &= ~BIT0;
            break;                               // CCR1 Set the pin to a 0
        case TB0IV_TBCCR2:
            P6OUT &= ~BIT6;
            break;                               // CCR2 Set the pin to a 0
        case TB0IV_TBIFG:
            P1OUT |= BIT0;                       //Set the pin to a 1
            P6OUT |= BIT6;                       //Set the pin to a 1
            break;
        default:
            break;                               //Reverts to no interupt
    }
}

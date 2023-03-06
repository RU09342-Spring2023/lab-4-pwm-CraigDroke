/*
 * HardwarePWM.c
 *
 *  Created on: Feb 18, 2023
 *      Author: Russell Trafford and Craig Droke
 *
 * This generates the same PWM as before but utlizes an external RGB LED. This additionally mixes colors to produce the ideal shade.
 */

#include <msp430.h>

unsigned int count = 1;

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;                         // Stop WDT

    //Configure output pins
    P6DIR |= BIT0;
    P6DIR |= BIT1;
    P6DIR |= BIT2;

    PM5CTL0 &= ~LOCKLPM5;

    //Timer_A init
    TB0CTL = TBSSEL__SMCLK | MC__UP | TBIE;           // Up mode clc (smclk)
    TB0CCTL1 |= CCIE;                                 // Enable TB0 CCR1 Interrupt
    TB0CCTL2 |= CCIE;                                 // Enable TB0 CCR1 Interrupt
    TB0CCR0 = 1000;                                   // Set CCR0 as period (1ms)
    TB0CCR1 = 30;                                     // Set CCR1
    TB0CCR2 = 930;                                    // Set CCR2

    //Setup Timer Compare IRQ
    TB1CCTL0 |= CCIE;                                // Enable TB1 CCR0 Overflow IRQ
    TB1CCR0 = 1000;
    TB1CTL = TBSSEL_1 | MC_2 | ID_3;                 // Cont mode (aclk)

    __bis_SR_register(LPM3_bits | GIE);              // Enter LPM3, enable interrupts
    __no_operation();

}

// Timer B1 interrupt service routine
#pragma vector = TIMER1_B0_VECTOR
__interrupt void Timer1_B0_ISR(void)
{
        if(count <= 300){
            TB0CCR1 += 3;                 //Increment the red LED brightness
            TB0CCR2 -= 3;                 //decrease blue LED brightness
        }
        else if(count > 300 && count <= 600){
            TB0CCR1 -= 3;                 //decrease red LED brightness
            TB0CCR2 += 3;                 //increase green LED Brightness
        }
        else if(count > 600 && count <= 900){
            TB0CCR1 += 3;                 //increase blue LED Brightness
            TB0CCR2 -= 3;                 //decrease green LED brightness
        }
        else if(count > 900 && count <= 1200){
            TB0CCR1 -= 3;                 //decrease blue LED brightness
            TB0CCR2 += 3;                 //increase red LED Brightness
        }
        else if(count > 1200 && count <= 1500){
            TB0CCR1 += 3;                 //increase green LED Brightness
            TB0CCR2 -= 3;                 //decrease red LED brightness
        }
        else if(count > 1500 && count <= 1800){
            TB0CCR1 -= 3;                 //decrease green LED brightness
            TB0CCR2 += 3;                 //increase blue LED Brightness
        }

        if(count > 1800){
            count = 1;                       //resets count
        }
        else{
        ++count;                            //increment count
        }

        TB1CCR0 += 30;                       //increase timer
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

            //Determine which LED to fade based on count
            if(count <= 300){
                P6OUT |= BIT1;      //Fade Blue LED
            }
            else if(count > 300 && count <= 900){
                P6OUT |= BIT2;      //Fade Green LED
            }
            else if(count > 900 && count <= 1500){
                P6OUT |= BIT0;      //Fade Red LED
            }
            else if(count > 1500 && count <= 1800){
                P6OUT |= BIT1;      //Fade Blue LED
            }
            break;
        case TB0IV_TBCCR2:
            //Determine which LED to fade based on count
            if(count <= 600){
                P6OUT |= BIT0;      //Fade Red LED
            }
            else if(count > 600 && count <= 1200){
                P6OUT |= BIT1;      //Fade Blue LED
            }
            else if(count > 1200 && count <= 1800){
                P6OUT |= BIT2;      //Fade Green LED
            }

            break;
        case TB0IV_TBIFG:
            P6OUT &= ~BIT0;   //Clear
            P6OUT &= ~BIT1;   //Clear
            P6OUT &= ~BIT2;   //Clear
            break;
        default:
            break;
    }
}

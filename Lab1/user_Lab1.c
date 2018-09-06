/******************************************************************************
MSP430F2272 Project Creator 4.0

ME 461 - S. R. Platt
Fall 2010

Updated for CCSv4.2 Rick Rekoske 8/10/2011

Written by: Steve Keres
College of Engineering Control Systems Lab
University of Illinois at Urbana-Champaign
 *******************************************************************************/

#include "msp430x22x2.h"
#include "UART.h"

char newprint = 0;
unsigned int timecnt = 0;
unsigned int fastcnt = 0;

char s_state = '0'; // switch state
char old_s_state = '0';

char switchstate(){

    switch((int)((P2IN^0x30)>>4)){ // Flip P2.4,5 P2IN as they are pulled up
    case 0:
        return '0';
    case 1:
        return '1';
    case 2:
        return '2';
    case 3:
        return '3';
    case 4:
        return '4';
    case 5:
        return '5';
    case 6:
        return '6';
    case 7:
        return '7';
    case 8:
        return '8';
    case 9:
        return '9';
    case 10:
        return 'A';
    case 11:
        return 'B';
    case 12:
        return 'C';
    case 13:
        return 'D';
    case 14:
        return 'E';
    case 15:
        return 'F';
    }
    return 'x'; // error signal
}


void main(void) {

    WDTCTL = WDTPW + WDTHOLD; // Stop WDT

    if (CALBC1_16MHZ ==0xFF || CALDCO_16MHZ == 0xFF) while(1);

    DCOCTL  = CALDCO_16MHZ; // Set uC to run at approximately 16 Mhz
    BCSCTL1 = CALBC1_16MHZ;

    //P1IN 		    Port 1 register used to use LEDs
    P1SEL &= ~0xFF; // Set all Port 1 pins to Port I/O function
    P1REN &= ~0xFF; // Disable internal resistor for all Port 1 pins
    P1DIR |= 0xFF;  // Set all Port 1 Pins as output
    P1OUT &= ~0xFF; // Initially set all Port 1 pins set as Outputs to zero
    P1IFG &= ~0xFF; // Clear Port 1 interrupt flags
    P1IES &= ~0xFF; // If port interrupts are enabled a High to Low transition on a Port pin will cause an interrupt
    P1IE  &= ~0xFF; // Disable all port interrupts

    //P2IN          Port 2 register used to read Port 2 pins 4,5,6,7 as Inputs
    P2SEL &= ~0xF0; // Set Port 2 pins 4,5,6,7 to Port I/O function
    P2REN |= 0xF0;  // Enable internal resistor for P2.4,5,6,7
    P2DIR &= ~0xF0; // Set Port 2 pins 4,5,6,7 as inputs
    P2OUT &= ~0xC0; // Pull down pins P2.6,7
    P2OUT |= 0x30;  // Pull up pins P2.4,5
    P2IFG &= ~0xFF; // Clear Port 2 interrupt flags
    P2IES &= ~0xFF; // If port interrupts are enabled a High to Low transition on a Port pin will cause an interrupt
    P2IE  &= ~0xFF; // Disable all port interrupts

    // Timer A Config
    TACCTL0 = CCIE;              // Enable Timer A interrupt
    TACCR0  = 16000;             // period = 1ms
    TACTL   = TASSEL_2 + MC_1;   // source SMCLK, up mode

    /* Uncomment this block when using the wireless modems on the robots
	// Wireless modem control pins
	P2SEL &= ~0xC0;	// Digital I/O
	P2REN &= ~0xC0; // Disable internal resistor for P2.6 and P2.7
	P2OUT |= 0x80;  // CMD/Data pin high for data
	P2DIR |= 0x80;	// Output direction for CMD/data pin (P2.7)
	P2DIR &= ~0x40;	// Input direction for CTS pin (P2.6)
	P2IFG &= ~0xC0; // Clear Port P2.6 and P2.7 interrupt flags
	P2IES &= ~0xC0; // If port interrupts are enabled a High to Low transition on a Port pin will cause an interrupt 
	P2IE &= ~0xC0;  // Disable P2.6 and P2.7 interrupts
	//Not sure why we are disabling P1.6 and P1.7 interrupts?
	P1IFG &= ~0xC0; // Clear Port P1.6 and P1.7 interrupt flags
	P1IES &= ~0xC0; // If port interrupts are enabled a High to Low transition on a Port pin will cause an interrupt 
	P1IE &= ~0xC0;  // Disable P1.6 and P1.7 interrupts
     */

    Init_UART(9600, 1);	// Initialize UART for 9600 baud serial communication

    _BIS_SR(GIE); 	    // Enable global interrupt


    while(1) {

        if(newmsg) {
            //my_scanf(rxbuff,&var1,&var2,&var3,&var4);
            newmsg = 0;
        }

        if (newprint)  {
            s_state = switchstate(); // find state of switches
            if (s_state != old_s_state)
                P1OUT = 0x00; // to sync all LEDs

            switch(s_state){
            case '0':
                P1OUT = 0xFF; // All ON
                break;
            case '1':
                P1OUT &= 0x01; // Turn OFF all other LEDs
                P1OUT ^= 0x01; // Blink LED
                break;
            case '2':
                P1OUT &= 0x02; // Turn OFF all other LEDs
                P1OUT ^= 0x02; // Blink LED
                break;
            case '3':
                P1OUT &= 0x03; // Turn OFF all other LEDs
                P1OUT ^= 0x03; // Blink LED
                break;
            case '4':
                P1OUT &= 0x04; // Turn OFF all other LEDs
                P1OUT ^= 0x04; // Blink LED
                break;
            case '5':
                P1OUT &= 0x05; // Turn OFF all other LEDs
                P1OUT ^= 0x05; // Blink LED
                break;
            case '6':
                P1OUT &= 0x06; // Turn OFF all other LEDs
                P1OUT ^= 0x06; // Blink LED
                break;
            case '7':
                P1OUT &= 0x07; // Turn OFF all other LEDs
                P1OUT ^= 0x07; // Blink LED
                break;
            case '8':
                P1OUT &= 0x08; // Turn OFF all other LEDs
                P1OUT ^= 0x08; // Blink LED
                break;
            case '9':
                P1OUT &= 0x09; // Turn OFF all other LEDs
                P1OUT ^= 0x09; // Blink LED
                break;
            case 'A':
                P1OUT &= 0x0A; // Turn OFF all other LEDs
                P1OUT ^= 0x0A; // Blink LED
                break;
            case 'B':
                P1OUT &= 0x0B; // Turn OFF all other LEDs
                P1OUT ^= 0x0B; // Blink LED
                break;
            case 'C':
                P1OUT &= 0x0C; // Turn OFF all other LEDs
                P1OUT ^= 0x0C; // Blink LED
                break;
            case 'D':
                P1OUT &= 0x0D; // Turn OFF all other LEDs
                P1OUT ^= 0x0D; // Blink LED
                break;
            case 'E':
                P1OUT &= 0x0E; // Turn OFF all other LEDs
                P1OUT ^= 0x0E; // Blink LED
                break;
            case 'F':
                P1OUT &= 0x0F; // Turn OFF all other LEDs
                P1OUT ^= 0x0F; // Blink LED
                break;
            case 'x':
                P1OUT = 0x00;  // All OFF when error
            }

            UART_printf("Switch State %c\n\r",s_state);
            old_s_state = s_state;

            timecnt++;  // Just incrementing this integer for default print out.
            newprint = 0;
        }

    }
}

// Timer A0 interrupt service routine
#pragma vector=TIMERA0_VECTOR
__interrupt void Timer_A (void)
{
    fastcnt++; // Keep track of time for main while loop.

    if (fastcnt == 500) {
        fastcnt = 0;

        newprint = 1;  // flag main while loop that .5 seconds have gone by.
    }

}


/*
// ADC 10 ISR - Called when a sequence of conversions (A7-A0) have completed
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void) {

}
 */


// USCI Transmit ISR - Called when TXBUF is empty (ready to accept another character)
#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void) {

    if((IFG2&UCA0TXIFG) && (IE2&UCA0TXIE)) { // USCI_A0 requested TX interrupt
        if(printf_flag) {
            if (currentindex == txcount) {
                senddone = 1;
                printf_flag = 0;
                IFG2 &= ~UCA0TXIFG;
            } else {
                UCA0TXBUF = printbuff[currentindex];
                currentindex++;
            }
        } else if(UART_flag) {
            if(!donesending) {
                UCA0TXBUF = txbuff[txindex];
                if(txbuff[txindex] == 255) {
                    donesending = 1;
                    txindex = 0;
                } else {
                    txindex++;
                }
            }
        }

        IFG2 &= ~UCA0TXIFG;
    }

    if((IFG2&UCB0TXIFG) && (IE2&UCB0TXIE)) { // USCI_B0 requested TX interrupt (UCB0TXBUF is empty)

        IFG2 &= ~UCB0TXIFG;   // clear IFG
    }
}


// USCI Receive ISR - Called when shift register has been transferred to RXBUF
// Indicates completion of TX/RX operation
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void) {

    if((IFG2&UCA0RXIFG) && (IE2&UCA0RXIE)) { // USCI_A0 requested RX interrupt (UCA0RXBUF is full)

        if(!started) {	// Haven't started a message yet
            if(UCA0RXBUF == 253) {
                started = 1;
                newmsg = 0;
            }
        } else { // In process of receiving a message
            if((UCA0RXBUF != 255) && (msgindex < (MAX_NUM_FLOATS*5))) {
                rxbuff[msgindex] = UCA0RXBUF;

                msgindex++;
            } else { // Stop char received or too much data received
                if(UCA0RXBUF == 255) { // Message completed
                    newmsg = 1;
                    rxbuff[msgindex] = 255;	// "Null"-terminate the array
                }
                started = 0;
                msgindex = 0;
            }
        }
        IFG2 &= ~UCA0RXIFG;
    }

    if((IFG2&UCB0RXIFG) && (IE2&UCB0RXIE)) { // USCI_B0 requested RX interrupt (UCB0RXBUF is full)

        IFG2 &= ~UCB0RXIFG; // clear IFG
    }

}




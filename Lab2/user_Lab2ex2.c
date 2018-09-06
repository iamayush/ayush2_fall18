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
// int period = 0; // period of blinking
// int TACCR0val = 16000; // for 1ms

void main(void) {
	
	WDTCTL = WDTPW + WDTHOLD; // Stop WDT
	
	if (CALBC1_16MHZ ==0xFF || CALDCO_16MHZ == 0xFF) while(1);
	                                             
	DCOCTL  = CALDCO_16MHZ; // Set uC to run at approximately 16 Mhz
	BCSCTL1 = CALBC1_16MHZ; 
		
	//P1IN 		    P1.2 as TA1
	P1SEL |= 0x04;
	P1REN &= ~0xFF;
	P1DIR |= 0x04;
	P1OUT &= ~0xFF;
	P1IFG &= ~0xFF;
	P1IES &= ~0xFF;
	P1IE  &= ~0xFF;

	// Timer A Config
	TACCTL0 = CCIE;              // Enable Timer A interrupt
	//TACCR0  = 50000;             // 40Hz
	TACCR0 = 5000;                 // 400 Hz
	TACTL   = TASSEL_2 + MC_1 + ID_3;   // source SMCLK, up mode, divider-8
	
	// PWM Config
	TACCTL1 = OUTMOD_7;
	//TACCR1 = 45000; // 5000-10%;45000-90%;25000-50%; for 40Hz
	TACCR1 = 2500; // 500-10%;4500-90%;2500-50%; for 400 Hz

	
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
			P1OUT ^= 0x04; // Blink LED
			//UART_printf("Period %d\n\r",period);
			// UART_send(1,(float)timecnt);
			
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




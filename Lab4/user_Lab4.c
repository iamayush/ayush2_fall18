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

// Create your global variables here:
int DACVAL = 0; // 10 bit value corresponding to desired voltage
int DACSEND = 0; // 16 bit value sent to TLV5606
char firstTX = 0; // indicates that 1st 8 bits have been sent
char rampPhase = 0; // 0:up, 1:down

// Functions
//void rampfunc(void); // generates ramp for DAC

void main(void) {

    WDTCTL = WDTPW + WDTHOLD; // Stop WDT

    if (CALBC1_16MHZ ==0xFF || CALDCO_16MHZ == 0xFF) while(1);

    DCOCTL  = CALDCO_16MHZ; // Set uC to run at approximately 16 Mhz
    BCSCTL1 = CALBC1_16MHZ;

    //P1IN  Blink LED at P1.0;
    P1SEL &= ~0xFF; // Set all Port 1 pins to Port I/O function
    P1REN &= ~0xFF; // Disable internal resistor for all Port 1 pins
    P1DIR |= 0x1;   // Set Port 1 Pin 0 (P1.0) as an output.  Leaves Port1 pin 1 through 7 unchanged
    P1OUT &= ~0xFF; // Initially set all Port 1 pins set as Outputs to zero

    //P3.1 as SIMO; P3.3 as SCLK; P3.0 as Output(FS) - SPI(USCIB0)
    //P3.6 as A6
    P3SEL |= 0x0A;
    P3SEL &= ~0x01;
    P3DIR |= 0x01;
    ADC10AE0 |= BIT6;

    // ADC Initializations
    ADC10CTL0 |= SREF_0 + ADC10SHT_1 + ADC10ON + ADC10IE;
    ADC10CTL1 |= INCH_6;


    //SPI Setup
    UCB0CTL0 |= UCMSB + UCMST + UCSYNC; // MSB first; master; synchronous
    UCB0CTL0 |= UCCKPH + UCCKPL; // Polarity & Phase;
    UCB0CTL1 |= UCSSEL_2 + UCSWRST; // SMCLK and Reset enabled
    UCB0BR0 = 16; // 1 MHz SPI Clock rate
    UCB0CTL1 &= ~UCSWRST;
    IE2 |= UCB0TXIE; // TX interrupt enabled
    IFG2 &= ~UCB0TXIFG; // clearing flag at initialization


    // Timer A Config
    TACCTL0 = CCIE;              // Enable Timer A interrupt
    TACCR0  = 1600;             // period = 0.1ms
    TACTL   = TASSEL_2 + MC_1;   // source SMCLK, up mode

    Init_UART(9600, 1);	// Initialize UART for 9600 baud serial communication

    _BIS_SR(GIE); 	    // Enable global interrupt

    while(1) {

        if(newmsg) {
            //my_scanf(rxbuff,&var1,&var2,&var3,&var4);
            newmsg = 0;
        }

        if (newprint)  {
            P1OUT ^= 0x1; // Blink LED
            UART_printf("Exp mV %d\n\r",(int)((DACVAL*3500L)/1023)); //  %d int, %ld long, %c char, %x hex form, %.3f float 3 decimal place, %s null terminated character array
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

    if (fastcnt == 5000) {
        fastcnt = 0;
        newprint = 1;  // flag main while loop that .5 seconds have gone by.
    }

    //    // Generating tone at 250 Hz
    //    if((fastcnt % 2) == 0){
    //        if(DACVAL == 0)
    //            DACVAL = 1023;
    //        else
    //            DACVAL = 0;
    //    }

    // ADC Sample at 10 kHz
    ADC10CTL0 |= ADC10SC + ENC;
}


// ADC 10 ISR - Called when a sequence of conversions (A7-A0) have completed
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void) {
    DACVAL = ADC10MEM; // echo ADC in thru DAC
    ADC10CTL0 &= ~ADC10IFG; // clear interrupt flag

    // Send value to DAC at 10 kHz
    DACSEND = ((DACVAL<<2) & 0x0FFF) | 0x4000; // DACSEND has 16 bits= bit0,1 as zero, bit2-11 as DACVAL
    // and bits12-15 for fast mode and normal power
    P3OUT |= 0x01;  // Frame select pulse
    P3OUT &= ~0x01; // at P3.0

    UCB0TXBUF = DACSEND>>8; // send MSB 8 bits first
    firstTX = 1; // flags that first 8 bits have been handled
}



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
        if(firstTX == 1){ // enter if MSB 8 bits have been handled
            UCB0TXBUF = DACSEND; //send LSB 8 bits now
            firstTX = 0; // reset flag for next transmit
        }
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
//void rampfunc(void){
//    if(rampPhase == 0){
//        DACVAL++;
//        if(DACVAL == 1023)
//            rampPhase = 1; // start down phase
//    }
//
//    else{
//        DACVAL--;
//        if(DACVAL == 0)
//            rampPhase = 0; // start up phase
//    }
//}



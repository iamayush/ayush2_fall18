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
int mvADCpot = 0; // ADC reading in mV
int mvADCi = 0; // ADC reading in mV
int rpm = 0; // motor speed from tachometer voltage
int pot_posn = 0; //link angular position thru pot
int i_posn = 0; // link ang posn via motor current (Vsense)
int ADCvals[8] = {0}; // [0] has A7(Curr2Motor), [5] has A2(Pot Angle Posn)
char dutycycle = 0; // PWM to Hbridge
char angle_deg = 0; // desired angle b/w -60 and 60 degrees

void drivePMDC(void);

void main(void) {

    WDTCTL = WDTPW + WDTHOLD; // Stop WDT

    if (CALBC1_16MHZ ==0xFF || CALDCO_16MHZ == 0xFF) while(1);

    DCOCTL  = CALDCO_16MHZ; // Set uC to run at approximately 16 Mhz
    BCSCTL1 = CALBC1_16MHZ;

    //P1IN 		    Port 1 register used to read Port 1 pins setup as Inputs
    P1SEL &= ~0xFF; // Set all Port 1 pins to Port I/O function
    P1REN &= ~0xFF; // Disable internal resistor for all Port 1 pins
    P1DIR |= 0xFF;   // Set Port 1 Pin 0 (P1.0) as an output.  Leaves Port1 pin 1 through 7 unchanged
    P1OUT &= ~0xFF; // Initially set all Port 1 pins set as Outputs to zero

    //P2IN          Port 2 register used to read Port 2 pins 4,5,6,7 as Inputs
    P2SEL &= ~0xF0; // Set Port 2 pins 4,5,6,7 to Port I/O function
    P2REN |= 0xF0;  // Enable internal resistor for P2.4,5,6,7
    P2DIR &= ~0xF0; // Set Port 2 pins 4,5,6,7 as inputs
    P2OUT &= ~0xC0; // Pull down pins P2.6,7
    P2OUT |= 0x30;  // Pull up pins P2.4,5

    // Timer A Config
    TACCTL0 = CCIE;              // Enable Timer A interrupt
    TACCR0  = 16000;             // period = 1ms
    TACTL   = TASSEL_2 + MC_1;   // source SMCLK, up mode

    // P4.4 as TB1; P4.7 as Output
    P4SEL |= 0x10;
    P4SEL &= ~0x80;
    P4DIR |= 0x90;
    P4OUT &= ~0x80;

    //debugging
    //    P4SEL |= 0x10;
    //    P4DIR |= 0x10;

    // Timer B Config
    TBCCR0 = 800; // 20 kHz
    TBCTL = TBSSEL_2 + MC_1; // SMCLK, up mode

    // PWM on P4.4 TB1
    TBCCTL1 = OUTMOD_7;
    TBCCR1 = 0;

    //    // ADC A2 at P2.2
    //    ADC10AE0 = BIT2;
    //    ADC10CTL0 |= ADC10SHT_2 + ADC10ON + ADC10IE;
    //    ADC10CTL1 |= INCH_2;

    ADC10AE0 = BIT2 + BIT7;
    ADC10CTL1 = INCH_7 + ADC10SSEL_3 + CONSEQ_1; // Enable A7 first, Use SMCLK, Sequence of Channels
    ADC10CTL0 = ADC10ON + MSC + ADC10IE;  // Turn on ADC, Put in Multiple Sample and Conversion mode,  Enable Interrupt
    ADC10DTC1 = 8;                 // Eight conversions.
    ADC10SA = (short)ADCvals;

    Init_UART(9600, 1);	// Initialize UART for 9600 baud serial communication

    _BIS_SR(GIE); 	    // Enable global interrupt

    while(1) {

        if(newmsg) {
            //my_scanf(rxbuff,&var1,&var2,&var3,&var4);
            newmsg = 0;
        }

        if (newprint)  {
            //P1OUT ^= 0x1; // Blink LED
            UART_printf("iX %d potX %d\n\r",i_posn,pot_posn);
            //UART_printf("mV %d rpm %d\n\r",mvADC,pot_posn); //  %d int, %ld long, %c char, %x hex form, %.3f float 3 decimal place, %s null terminated character array
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
    if (fastcnt == 250) {
        fastcnt = 0;
        newprint = 1;  // flag main while loop that .25 seconds have gone by.
    }

    // Put your Timer_A code here:
    drivePMDC();
    //TBCCR1 = 17*8;
    ADC10CTL0 |= ENC + ADC10SC;
}



// ADC 10 ISR - Called when a sequence of conversions (A7-A0) have completed
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void) {
    mvADCpot = (ADCvals[5]*3600L)/1023; // in mV
    mvADCi = (ADCvals[0]*3600L)/1023; // in mV
    // rpm = ((-25*mvADC)/10) + 3798; // calibrated .xlsx
    pot_posn = (mvADCpot/10) - 100; // in degrees: pos=100*V-200
    i_posn = (int)((27303L*mvADCi)/1000000) - 3; // in degrees thru motor current or Vsense

    // for next call of ADC ISR
    ADC10CTL0 &= ~ADC10IFG;  // clear interrupt flag
    ADC10SA = (short)ADCvals; // ADC10 data transfer starting address
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

void drivePMDC(){

    switch((int)(P2IN>>6)){ // Pins 2.6 and 2.7
    case 0:
        angle_deg = 0;
        P1OUT |= 0xFF;
        break;
    case 1:
        angle_deg = 30;
        P1OUT = 0x1;
        break;
    case 2:
        angle_deg = 45;
        P1OUT = 0x2;
        break;
    case 3:
        angle_deg = 60;
        P1OUT = 0x3;
        break;
    }

    dutycycle = (char)((1711L*angle_deg)/1000) + 2;
    TBCCR1 = dutycycle*8;

    if (P2IN & 0x10){
        P4OUT &= ~0x80;
        P1OUT |= 0x80;
    }
    else{
        P4OUT |= 0x80;
        P1OUT &= ~0x80;
    }
}


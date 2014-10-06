// ******************************************************************************************* //
// Include file for PIC24FJ64GA002 microcontroller. This include file defines
// MACROS for special function registers (SFR) and control bits within those
// registers.

#include "p24fj64ga002.h"
#include <stdio.h>
#include "lcd.h"

// ******************************************************************************************* //
// Configuration bits for CONFIG1 settings. 
//
// Make sure "Configuration Bits set in code." option is checked in MPLAB.
//
// These settings are appropriate for debugging the PIC microcontroller. If you need to 
// program the PIC for standalone operation, change the COE_ON option to COE_OFF.

_CONFIG1( JTAGEN_OFF & GCP_OFF & GWRP_OFF & 
          BKBUG_ON & COE_ON & ICS_PGx1 & 
          FWDTEN_OFF & WINDIS_OFF & FWPSA_PR128 & WDTPS_PS32768 )

// ******************************************************************************************* //
// Configuration bits for CONFIG2 settings.
// Make sure "Configuration Bits set in code." option is checked in MPLAB.

_CONFIG2( IESO_OFF & SOSCSEL_SOSC & WUTSEL_LEG & FNOSC_PRIPLL & FCKSM_CSDCMD & OSCIOFNC_OFF &
          IOL1WAY_OFF & I2C1SEL_PRI & POSCMOD_XT )

// ******************************************************************************************* //
// Defines to simply UART's baud rate generator (BRG) regiser
// given the osicllator freqeuncy and PLLMODE.

#define XTFREQ          7372800         	  // On-board Crystal frequency
#define PLLMODE         4               	  // On-chip PLL setting (Fosc)
#define FCY             (XTFREQ*PLLMODE)/2    // Instruction Cycle Frequency (Fosc/2)

#define BAUDRATE         115200       
#define BRGVAL          ((FCY/BAUDRATE)/16)-1 

// ******************************************************************************************* //

// Varaible utilized to store the count that is incremented within the Timer 1 interrupt 
// service routine every second.
// Notes:
//        1. This variable is declared are volatile becuase it is updated in the interrupt 
//           service routine but will be read in the main execution loop.
//        2. Declared as unsigned char as the varaible will only store the values between 
//           0 and 10.
volatile unsigned char cnt;

// ******************************************************************************************* //

int main(void)
{
	// RPINR18 is a regsiter for selectable input mapping (see Table 10-2) for 
	// for UART1. U1RX is 8 bit value used to specifiy connection to which
	// RP pin. RP9 is used for this configuration. Physical Pin 18.
	RPINR18bits.U1RXR = 9;	

	// RPOR4 is a register for selctable ouput mapping (see Regsiter 1019) for
	// pins RP9 and RP8. The register for RP8 is assigned to 3 to connect 
	// the U1TX output for UART1 (see table 10-3). Physical Pin 17.
	RPOR4bits.RP8R = 3;		

	// Set UART1's baud rate generator register (U1BRG) to the value calculated above.
	U1BRG  = BRGVAL;

	// Set UART1's mode register to 8-bit data, no parity, 1 stop bit, enabled.
	//     UARTEN        = 1     (enable UART)
	//     PDSEL1:PDSEL0 = 00    (8-bit data, no parity)
	//     STSEL         = 0     (1 stop bit)
	U1MODE = 0x8000; 		

	// Set UART2's status and control register
	//     UTXISEL1:UTXISEL0 = 00    (U1TXIF set when character 
	//                                written to trasmit buffer)
	//     UTXEN             = 1     (trasnmit enabled)
	//     URXISEL1:URXISEL0 = 01    (U1RXIF set when any character 
	//                                is received in receive buffer)
	//     RIDLE             = 0     (Reciver is active)
	U1STA  = 0x0440; 		// Reset status register and enable TX & RX

	// Clear the UART RX interrupt flag. Althouhg we are not using a ISR for 
	// the UART receive, the UART RX interrupt flag can be used to deermine if 
	// we have recived a character from the UART. 
	IFS0bits.U1RXIF = 0;

	// The following provides a demo configuration of Timer 1 in which
	// the Timer 1 interrupt service routine will be executed every 1 second 
	PR1 = 57599;			
	TMR1 = 0;				
	IFS0bits.T1IF = 0;		
	IEC0bits.T1IE = 1;
	T1CONbits.TCKPS = 3;
	T1CONbits.TON = 1;

	// printf by default is mapped to serial communication using UART1.
	// NOTES:
	//        1. You must specify a heap size for printf. This is required
	//           becuase printf needs to allocate its own memory, which is
	//           allocated on the heap. This can be set in MPLAB by:
	//           a.) Selecting Build Options...->Project from the Project menu.
	//           b.) Selecting the MPLABLINK30 Tab.
	//           c.) Entering the size of heap, e.g. 512, under Heap Size
	//        2. printf function is advanced and using printf may require 
	//           significant code size (6KB-10KB).   
	printf("Lab 2: Debugging Statements\n\r");

	// The following code will not work until you have implemented the 
	// the required LCD functions defined within lcd.c
	LCDInitialize();
	LCDPrintString("Running:");
	LCDMoveCursor(1,0);
	LCDPrintChar('0');
	LCDPrintChar('0');
	LCDPrintChar(':');
	LCDPrintChar('0');
	LCDPrintChar('0');
	LCDPrintChar('.');
	LCDPrintChar('0');
	LCDPrintChar('0');

	while(1)
	{
		LCDMoveCursor(1,4);
		LCDPrintChar(cnt+'0');		
	}
	return 0;
}

// ******************************************************************************************* //
// Defines an interrupt service routine that will execute whenever Timer 1's
// count reaches the specfied period value defined within the PR1 register.
// 
//     _ISR and _ISRFAST are macros for specifying interrupts that 
//     automatically inserts the proper interrupt into the interrupt vector 
//     table
//
//     _T1Interrupt is a macro for specifying the interrupt for Timer 1
//
// The functionality defined in an interrupt should be a minimal as possible
// to ensure additional interrupts can be processed. 
void _ISR _T1Interrupt(void)
{
	// Clear Timer 1 interrupt flag to allow another Timer 1 interrupt to occur.
	IFS0bits.T1IF = 0;		
	
	// Updates cnt to wraparound from 9 to 0 for this demo.
	cnt = (cnt<9)?(cnt+1):0;
}

// ******************************************************************************************* //
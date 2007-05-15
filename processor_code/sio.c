/*******************************************************************************
 * RS232 serial IO routines
 ******************************************************************************
 * original version: RHJ 1-May-06
 *
 *
 */

#define  SIO

#include <AT89c5131.h>
#include <stdio.h>
#include <ctype.h>
#include <intrins.h>

#include "SyncoCmd.h"


/*---------- Global variables */
char sio_rx_data = 'r';
char sio_rxbuf[RXBUFSIZ];
char sio_rx_idx = 0;
bit	 sio_rx_gotcl = 0;

/*------------------------------------------------------------------------*/

/*----- putchar (mini version): outputs charcter only */
char 
putchar(char c)  
{
	while (!TI); 		// wait for any current tx to end; until TI goes high
	TI = 0;
	return (SBUF = c);
}


/** Version 2
 * FUNCTION_PURPOSE: serial interrupt, echo received data.
 * FUNCTION_INPUTS: P3.0(RXD) serial input
 * FUNCTION_OUTPUTS: P3.1(TXD) serial output
 */
void serial_IT(void) interrupt 4 
{
if ( RI == 1 ) 
	{
	RI = 0;										// Clears RI Interrupts
	sio_rx_data = SBUF;							// Read receive data
	
	if ( isprint(sio_rx_data) )
		{
		TI = 0;									// clear tx flag for next emission 
		SBUF = sio_rx_data;						// echo rx data on tx
		sio_rxbuf[sio_rx_idx++] = sio_rx_data;	// add it to the buffer
		}
		
	if (sio_rx_idx >= (RXBUFSIZ-1))				// prevent buffer overrun
		{	--sio_rx_idx; }
	if (sio_rx_data == '\b' && sio_rx_idx > 0)	// if 'backspace' go back 1
		{ 
		TI = 0;									// clear tx flag for next emission 
		SBUF = sio_rx_data;						// echo the backspace	
		sio_rxbuf[sio_rx_idx] = 0;				// delete previous char from buffer
		sio_rx_idx -= 1; 						// and change the index
		}
	else if (sio_rx_data == '\r') 				// if EOL
		{
		sio_rxbuf[sio_rx_idx] = 0;				// reset stuff
		sio_rx_idx = 0;
		sio_rx_gotcl = 1;						// set flag that we have a cmd line
		}
	}   
	else	// if Tx interrupt
		{
		;
		}								
}




/* FUNCTION_PURPOSE: This file set up uart in mode 1 (8 bits uart) with
 * internal baud rate generator.
 */
void 
sio_Init_9600(void)
{
	SCON    = 0x50;				// uart in mode 1 (8 bit), REN=1
	BDRCON &= 0xEC;				// BRR=0; SRC=0;
	BDRCON |= 0x0e;				// TBCK=1; RBCK=1; SPD=1
	PCON   |= 0x80;				// SMOD1 = 1, double baud rate
	BRL	= 0x64;					// 9600 Bds at 24.00MHz
	CKCON0 = 0x7f;
	ES = 1;						// Enable serial interrupt
	EA = 1;						// Enable global interrupt
	BDRCON |= 0x10;				// BRR=1; Baud rate generator run
	TI = 1 ;
}

/*======================================================================================*/


/*----- Timer0 Setup 
// Interrupt occurs every ... when enabled
void 
Timer0_Init(void)
{
	TMOD = 0x01;
	ET0 = ON;          			// Enable Timer0 Interrupts
	TR0 = ON;					// Timer0 Run
}
*/

//sbit PIO_TST =	P1^6;			// output for testing

/*----- Timer0 Service Routine 
// Interrupt occurs every 25 ms when enabled
void 
Timer0_isr (void) interrupt 1 using 3
{
int i;

TH0 = 0x3C;
TL0 = 0xC2;
// stretched output for test
PIO_TST = (bit)0;
for(i=0; i<33; i++);
PIO_TST = (bit)1;
}
*/

/************************** EOF **********************************************/


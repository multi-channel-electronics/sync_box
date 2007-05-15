/*==========================================================================* 
 * piolib.c																	* 
 * interface to the Mancho CPLD. 										* 
 *==========================================================================* 
 * First version: RHJ 9-June-06
 *
 *
 */

#define  PIOLIB

#include <AT89c5131.h>
#include <stdio.h>
#include <intrins.h>

#include "SyncoCmd.h"


/*---------- Function prototypes */
void pio_Reset(bit b);
void pio_nEnable(bit b);
void pio_SyncLength(unsigned long sl); 
void pio_FrameNum(unsigned long fn);
void pio_DV_Mode(unsigned char mode);
void pio_FRun_Count(int frc);
//void pio_RdSwitches();
void pio_pwr_onoff(unsigned char enbits);
unsigned char pio_pwr_status(void);
void pio_Chk_PSCool();

/*=========================================================================================*/
/*---------- PIO Interface Defines; See also PIO_Interface.vhd */
/* 
P1.0 		= PIO_Reset
P1.1 		= PIO_nEnable
P1.2 to P1.5 = switches input [not used]
P1.6 		=  [not used]
P1.7 		=  Spare opto-isolated TTL Input. [not used]
	-----------------
P3.0 		= RS232_RXD, 
P3.1 		= RS232_TXD, 
P3.2 to P3.5 = Interrupts & timers, 
P3.6 		= PIO_nWR, 
P3.7 		= PIO_nRD. 
*/

#define PIO_DAT	P0
#define PIO_ADR	P2

sbit PIO_RESET	=	P1^0;		// outputs init high, so initial state is reset,
sbit PIO_nENABLE =	P1^1;		// and clocking disabled.
sbit PIO_SW4	=	P1^2;		// [not used]
sbit PIO_SW3	=	P1^3;		// [not used]
sbit PIO_SW2	=	P1^4;		// [not used]
sbit PIO_SW1	=	P1^5;		// [not used]
sbit PIO_PSCOOL =	P1^7;		// [not used] [was 'cooling interlock', is now 'spare opto-TTL input']
//
sbit PIO_nINT0	=	P3^2;		// [not used]
sbit PIO_nINT1	=	P3^2;		// [not used]
sbit PIO_T0		=	P3^4;		// [not used]
sbit PIO_T1		=	P3^5;		// [not used]
sbit PIO_nWR	=	P3^6;		// 
sbit PIO_nRD	=	P3^7;		//

// PIO CPLD/Firmware register address definitions. See also PIO_Interface.vhd 
#define  ADR_xxx 		00 // -- X"0000"	xxx
#define  ADR_CMDSEL 	01 // -- X"0001"	Cmd function Select
#define  ADR_MODEREG	02 // -- X"0002"	Mode Control Register
#define  ADR_CMDDATB0	16 // -- X"0010"	CmdData[7..0]
#define  ADR_CMDDATB1	17 // -- X"0011"	CmdData[15..8]
#define  ADR_CMDDATB2	18 // -- X"0012"	CmdData[23..16]
#define  ADR_CMDDATB3	19 // -- X"0013"	CmdData[31..24]
#define  ADR_AUXO		32 // -- X"0020"	AuxOut
#define  ADR_AUXI		33 // -- X"0021"	AuxIn
//
// CmdSelect reg control vaules
#define  SLEN_LOAD		1	// Load SyncLength counter
#define  FRUN_LOAD		2	// FreeRun counter
#define  DVCNTR_LOAD	4	// DV, FrameNumber, counter
//
// Mode Control Select Reg., RTS_DV or FRUN_DV control bit; 1 = FR_Enable
#define  MODE_DV		00 


/*-------------------------------------------------------------------------------------------*/
/* To write to a Mancho-CPLD reg, first write the value bytes into the CmdData reg, 
 * then write the appropriate select byte to the CmdSelect register.
 */
//	if (LowAddr = ADR_CMDSEL and PIO_A = X"00" and PIO_nWR = '0') then 
//		CmdWrite <= '1';
//	else 
//		CmdWrite <= '0';
//	end if;
//	SL_Load      <= CmdWrite and match(PIO_AD, (X"01"));	-- 01
//	FR_Load      <= CmdWrite and match(PIO_AD, (X"02"));	-- 10
//	DV_Cntr_Load <= CmdWrite and match(PIO_AD, (X"03")); 	-- 11;
//
//  bit 0 of the ModeReg controls whether FreeRun or RTS DV is used.
//	FR_Enable <= ModeReg(0);


/*=========================================================================================*/
/* The 8051 is big-endian, 
 * the cpld will be addressed as little-endian, 
 * so the byte order must be reversed when writing to the CPLD CMDDATA.
 *
 * In this version of SyncoCmd, P0 is used for data io, P2 for an 8 bit address
 */

union
     {
     unsigned long l_cnt;
     int i_cnt;
     unsigned char bytes[4];
     }  piodata;

#define PDB0 piodata.bytes[0]
#define PDB1 piodata.bytes[1]
#define PDB2 piodata.bytes[2]
#define PDB3 piodata.bytes[3]


 
 /*-------------------------------------------------------------------------------------------*/

/*-----  */
void
pio_Reset(bit b)
{
PIO_RESET = b;		// 
}

/*-----   */
void 
pio_nEnable(bit b)
{
if(b==ON) 
	PIO_nENABLE = 0;		// note that nEnable is active low
else 
	PIO_nENABLE = 1;
} 
 
/*----- set the sync length count */
void
pio_SyncLength(unsigned long sl)  
{
piodata.l_cnt = sl;

//printf("\tSL = %d", piodata.i_cnt);

// first load the count to the CmdData reg.
PIO_ADR = ADR_CMDDATB0;
PIO_DAT = PDB3;
_nop_ ();
PIO_nWR = LOW;
PIO_nWR = HIGH;
PIO_ADR = ADR_CMDDATB1;
PIO_DAT = PDB2;
_nop_ ();
PIO_nWR = LOW;
PIO_nWR = HIGH;
PIO_ADR = ADR_CMDDATB2;
PIO_DAT = PDB1;
_nop_ ();
PIO_nWR = LOW;
PIO_nWR = HIGH;
PIO_ADR = ADR_CMDDATB3;
PIO_DAT = PDB0;
_nop_ ();
PIO_nWR = LOW;
PIO_nWR = HIGH;
//
PIO_ADR = ADR_CMDSEL;	// then load the CmdSelect reg
PIO_DAT = SLEN_LOAD;	// select the SyncLength load reg
_nop_ ();
PIO_nWR = LOW;
PIO_nWR = HIGH;
PIO_ADR = 0xFF;			// put back to high impedance.
PIO_DAT = 0xFF;
}


/*----- Set the DV_Count/FrameNumber counter */
void
pio_FrameNum(unsigned long fn)
{
piodata.l_cnt = fn;

PIO_ADR = ADR_CMDDATB0;
PIO_DAT = PDB3;
_nop_ ();
PIO_nWR = LOW;
PIO_nWR = HIGH;
PIO_ADR = ADR_CMDDATB1;
PIO_DAT = PDB2;
_nop_ ();
PIO_nWR = LOW;
PIO_nWR = HIGH;
PIO_ADR = ADR_CMDDATB2;
PIO_DAT = PDB1;
_nop_ ();
PIO_nWR = LOW;
PIO_nWR = HIGH;
PIO_ADR = ADR_CMDDATB3;
PIO_DAT = PDB0;
_nop_ ();
PIO_nWR = LOW;
PIO_nWR = HIGH;
//
PIO_ADR = ADR_CMDSEL;	// load the CmdSelect reg
PIO_DAT = DVCNTR_LOAD;	// select the FrameCounter load reg
_nop_ ();
PIO_nWR = LOW;
PIO_nWR = HIGH;
PIO_ADR = 0xFF;
PIO_DAT = 0xFF;
}


/*----- Set the Mode byte */
void
pio_DV_Mode(unsigned char mode)
{

PIO_ADR = ADR_MODEREG;	// one byte, write it direct to reg.
PIO_DAT = mode;
_nop_ ();
PIO_nWR = LOW;
PIO_nWR = HIGH;
PIO_ADR = 0xFF;
PIO_DAT = 0xFF;
}


/*-----  Set the FreeRun count-compare reg. 
		[the FR counter increments on AddrZero from SyncLength counter] */
void
pio_FRun_Count(int frc)
{
piodata.i_cnt = frc;

// load count to the CmdData reg; only 2 bytes used.
PIO_ADR = ADR_CMDDATB0;
PIO_DAT = PDB1;
_nop_ ();
PIO_nWR = LOW;
PIO_nWR = HIGH;
PIO_ADR = ADR_CMDDATB1;
PIO_DAT = PDB0;
_nop_ ();
PIO_nWR = LOW;
PIO_nWR = HIGH;
//
PIO_ADR = ADR_CMDSEL;	// load the CmdSelect reg
PIO_DAT = FRUN_LOAD;	// select the FreeRun counter load reg
_nop_ ();
PIO_nWR = LOW;
PIO_nWR = HIGH;
PIO_ADR = 0xFF;
PIO_DAT = 0xFF;
}


/*----- Set the ACDCUs on/off control byte   */
void pio_pwr_onoff(unsigned char enbits)
{
PIO_ADR = ADR_AUXO;
PIO_DAT = enbits;
_nop_ ();
_nop_ ();			// give some extra time for things to propagate thru the CPLD to the external reg.
PIO_nWR = LOW;
PIO_nWR = HIGH;
PIO_ADR = 0xFF;
PIO_DAT = 0xFF;
}

/*----- Read the ACDCUs status byte */
unsigned char
pio_pwr_status(void)
{
unsigned char stat;

PIO_ADR = ADR_AUXI;
PIO_nRD = LOW;
_nop_ ();
_nop_ ();			// give some extra time for things to propagate thru the CPLD from the external reg.
_nop_ ();			// and for bus to settle
_nop_ ();
stat = PIO_DAT;
PIO_nRD = HIGH;
PIO_ADR = 0xFF;
PIO_DAT = 0xFF;

return stat;
}


/*----- not used
void
pio_RdSwitches()
{
bit sw1, sw2, sw3, sw4;

sw1 = PIO_SW1;
sw2 = PIO_SW2;
sw3 = PIO_SW3;
sw4 = PIO_SW4;

//  not finished, needs to return the switch value
}
 */


/*----- not used 
void 
pio_Chk_PSCool()
{
bit iscool = PIO_PSCOOL;

if((iscool = PIO_PSCOOL) == 0 )
	printf("\r\tCOOLING FAIL?");
}
*/


/************************** EOF **********************************************/


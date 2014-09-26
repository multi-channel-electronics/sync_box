/*==========================================================================*
 * SyncoCmd.c																*
 * main() and  command io routines. 										*
 *==========================================================================*
 * First Version for SC2: SyncoCmd-V1a RHJ 1-May-06
 * SyncoCmd-V1b RHJ 17-Aug-06	- minor cosmetic changes and additions.
 * 22-Oct-06 RHJ 				- added pwr_status() command function.
 * 2010-Feb-25 MA               - adds ckd command to specify a 50MHz divisor
 *                              - (1 to 255) to generate frequencies for
 *                              - DV_Spare1 and DV_Spare2 outputs.
 * 2013-Aug-22 MA               - hard-code Spider values for fr=120,
 *                              - row_len=53, num_rows=33
 */

#define  SYNCOMAIN

#include <AT89c5131.h>
#include <stdio.h>
#include <intrins.h>
#include "SyncoCmd.h"



/*---------- Function prototypes */

void prt_toosmall(int i);
void prt_toobig(int i);
void prt_minsync(void);
void chk_switches(void);
//
void do_ResetAll(void);
void set_Enable(void);
void set_Disable(void);
void get_Status(void);
void set_Row_Len(void);
void set_Num_Rows(void);
void set_FR_Mode(void);
void set_RTS_Mode(void);
void set_Frame_Num(void);
void set_clk_adj_div(void);
void pwr_enable(void);
void pwr_disable(void);
void pwr_status(void);


/*=============================================================================================*/
/*---------- Variable Value Defines */
#define  FRUN_DV		(unsigned char)1
#define  RTS_DV			(unsigned char)0
//
// Default values for start & reset
//#define  FRUN_COUNT		3	// This group for test.
//#define  ROWLEN			64
//#define  NUMROW			2
//

#define  FRUN_COUNT		38	// ACT specific
#define  ROWLEN			50  // ACT specific
#define  NUMROW			33  // ACT specific

//#define  FRUN_COUNT		47	// about 200 Hz DV rate
//#define  ROWLEN			64
//#define  NUMROW			41
#define  CLKADJDIV      10  // default divisor for adjustable clk frequency clk_adj_div

// min-max values for cmd input
#define  MINROWLEN		 1	// rowlen setable 50 -> 4095
#define  MAXROWLEN	  4095
#define  MINNUMROW		 1	// numrow setable 1 -> 63
#define  MAXNUMROW		63
#define  MINSYNCLEN		250	// rowlen * numrow must be > 250
#define  MINFRCNT		 1	// DV_FRUN output 1 to 4095 occurances of AddrZero
#define  MAXFRCNT	  4095
#define  MINDIV       1     // Minimum 50MHz division ratio for adjustable clk outputs
#define  MAXDIV       255   // Maximum 50MHz division ratio for adjustable clk outputs


/*=============================================================================================*/
/*---------- Global variables---------------*/
/*  FOR STARTUP DEFAULTS, SEE: do_ResetAll() */
bit sc_mancho_enable;			// flag bit for whether manchester output is running or not
int sc_FRun_Count;				// AddrZero count for output of a DV_FreeRun
int sc_row_len;					// Default Row_Length
unsigned char sc_num_row;		// Default SyncLength = (num_row * row_len) - 1
unsigned char sc_mancho_mode;	// mode control byte; is DV source DV_FreeRun or DV_RTS
unsigned char sc_ACDCU_onoff = 0;	// all ACDCCU off
int sc_clk_adj_div;             // Default divisor for the adjustable clock frequency

char code version[] =  "\r\tSyncoCmd-V22\r";
char code prompt[] = "\rSynco> ";

/*=============================================================================================*/
//--------
void do_ResetAll(void)
{
//sc_mancho_mode = RTS_DV;
sc_mancho_mode = FRUN_DV;
sc_FRun_Count = FRUN_COUNT;
sc_row_len = ROWLEN;
sc_num_row = NUMROW;
sc_mancho_enable = TRUE;	//
sc_clk_adj_div = CLKADJDIV;
//
pio_nEnable(OFF);	//
pio_Reset((bit)ON);
pio_Reset((bit)OFF);
pio_SyncLength((sc_row_len*sc_num_row)-1);
pio_FRun_Count(sc_FRun_Count-1);
pio_clk_adj_div(sc_clk_adj_div);
pio_DV_Mode(sc_mancho_mode);
pio_FrameNum((unsigned long)0);
pio_nEnable(sc_mancho_enable);	//
}

/*=============================================================================================*/

void main (void)
{

sio_Init_9600();
TI = 1 ;
//Timer0_Init();	// not used. code is in sio.c
//
printf("%s", version);
cd_help();
printf("%s", prompt);
do_ResetAll();

while(1)			// endless
	{
//	testxx();		// test & debug stuff.

	if(sio_rx_gotcl == TRUE)
		{									// got a cmd line
		sio_rx_gotcl = FALSE;
		ES = OFF;							// disable serial interrupt
		cd_tokenize(sio_rxbuf);				// parse a command line
		cd_parse(cmd_dict, dict_size);
		sio_rx_idx = 0;						// reset message pointer
		ES = ON;							// Enable serial interrupt
		printf("%s", prompt) ;
		}
//	chk_switches();
	}
}


/*========== Utility Functions for command variable range errors =====================*/

//--------
void prt_needarg(void)
{ printf("\tARGUMENT REQUIRED"); }

//--------
void prt_toosmall(int i)
{ printf("\tTOO SMALL \"%d\"", i); }

//--------
void prt_toobig(int i)
{ printf("\tTOO BIG \"%d\"", i); }

//--------
void prt_minsync(void)
{ int i = MINSYNCLEN; printf("\tROWLEN * NUMROW MUST BE >= %d", i); }


/*========== Command Dictionary Functions  ===============================*/


//--------
void set_Enable(void)
{
sc_mancho_enable = TRUE;
pio_nEnable(ON);
}

//--------
void set_Disable(void)
{
sc_mancho_enable = FALSE;
pio_nEnable(OFF);
}

//--------
void get_Status(void)
{
code char fr_str[] = "FreeRun_DV";
code char rt_str[] = "RTS_DV";
char *dvm;
code char ena_str[] = "ON";
code char dis_str[] = "OFF";
char *is_run;

if(sc_mancho_enable == TRUE)
	is_run = ena_str;
else
	is_run = dis_str;

if(sc_mancho_mode == FRUN_DV)
	dvm = fr_str;
else
	dvm = rt_str;

putchar('\r');
printf("\tMancho_Enable\t= %s\r", is_run);
printf("\tDV_Mode\t\t= %s\r", dvm);
if(sc_mancho_mode == FRUN_DV)
	printf("\tFRun_Count\t= %d\r", sc_FRun_Count);
printf("\tRow_Len\t\t= %d\r", sc_row_len);
printf("\tNum_Row\t\t= %d\r", (int)sc_num_row);
printf("\tAdjustable Clock frequency divisor\t\t=%d\r", sc_clk_adj_div);
printf("\tACDCU_onoff\t= %#2.2X\r", (int)sc_ACDCU_onoff);

}

//--------
void set_Row_Len(void)
{
int rl;
unsigned long sl;

rl = cd_arg_i();
if(rl == -2 ) {prt_needarg(); return;}	// if rl = -2 [= no arg], then error msg
if(rl < 0 ) {return;}					// if rl = -1 [= not a digit arg], then just ignor
if(rl < MINROWLEN ) {prt_toosmall(rl); return;}
if(rl > MAXROWLEN ) {prt_toobig(rl); return;}
sl = (long)rl * (long)sc_num_row;
if(sl < MINSYNCLEN ) {prt_minsync(); return;}

sc_row_len = rl;					// everything ok, set as the new row_len
pio_nEnable(OFF);					//
pio_SyncLength(sl - 1); 			// minus 1 because counter includes zero
pio_nEnable(sc_mancho_enable);		//
}

//--------
void set_Num_Rows(void)
{
int nr;
unsigned long sl;

nr = cd_arg_i();
if(nr == -2 ) {prt_needarg(); return;}
if(nr < 0 ) {return;}
if(nr < MINNUMROW ) {prt_toosmall(nr); return;}
if(nr > MAXNUMROW ) {prt_toobig(nr); return;}
sl = (long)nr * (long)sc_row_len;
if(sl < MINSYNCLEN ) {prt_minsync(); return;}

sc_num_row = (unsigned char)nr;	//
pio_nEnable(OFF);				//
pio_SyncLength(sl - 1); 		// minus 1 because counter includes zero
pio_nEnable(sc_mancho_enable);	//
}

//--------
void set_FR_Mode(void)
{
int frc;

frc = cd_arg_i();
//printf("\targ = %d\r", frc);
if(frc >= 0)
	{
	if(frc < MINFRCNT ) {prt_toosmall(frc); return;}
	if(frc > MAXFRCNT ) {prt_toobig(frc); return;}
	sc_FRun_Count = frc;
	}

// if arg = -1 [= not a digit arg], then just ignor
// if arg = -2 [= no arg], then use previous sc_FRUN_COUNT
if(frc < 0)
	{
	if(frc == -1)
		return;
	else
		frc = sc_FRun_Count;
	}

sc_mancho_mode = FRUN_DV;			//
pio_nEnable(OFF);					//
pio_FRun_Count(frc-1);				// minus 1 because counter includes zero
//pio_DV_Mode(sc_mancho_mode);		// maybe sometime use the other mode bits for extra functions
pio_DV_Mode((unsigned char)0xff);	// but for now we will flip all bits
pio_nEnable(sc_mancho_enable);		//
}

//-------- switch to RTS_DV mode
void set_RTS_Mode(void)
{
sc_mancho_mode = RTS_DV;
pio_nEnable(OFF);					//
//pio_DV_Mode(sc_mancho_mode);		//
pio_DV_Mode((unsigned char)0x00);	//
pio_nEnable(sc_mancho_enable);		//
}

//-------- set the Frame_Number/DV_Count
void set_Frame_Num(void)
{
int r;
unsigned long fn;

r = cd_arg_ul(&fn);
if(r == -2 ) {prt_needarg(); return;}
if(r < 0) return;

//printf("\tfn = %lu", fn);
pio_nEnable(OFF);				//
pio_FrameNum(fn);
pio_nEnable(sc_mancho_enable);	//
}
//-------- set the 50MHz/div for DV_SPARE2 output
void set_clk_adj_div(void)
{
int r;
int clk_adj_div_temp;

clk_adj_div_temp = cd_arg_i();
if(clk_adj_div_temp >= 0)
	{
	if(clk_adj_div_temp < MINDIV ) {prt_toosmall(clk_adj_div_temp); return;}
	if(clk_adj_div_temp > MAXDIV ) {prt_toobig(clk_adj_div_temp); return;}
	sc_clk_adj_div = clk_adj_div_temp;
	}

// pio_nEnable(OFF);				//
pio_clk_adj_div(sc_clk_adj_div);
// pio_nEnable(sc_mancho_enable);	//
}
//-------- enable a particular ACDCU [sequencing all units on is left up to the user]
void pwr_enable_unit(void)
{
int unit_num;
unsigned char unit_bit = 1;
unsigned char f;

unit_num = cd_arg_i();
if(unit_num == -2 ) {prt_needarg(); return;}
if(unit_num < 0 ) return;
if(unit_num > 7 ) {prt_toobig(unit_num); return;}

unit_bit = (unit_bit<<unit_num);	// shift bit into the proper position
f = (sc_ACDCU_onoff & unit_bit);
//printf("\r\tonoff = %#2.2X, ub = %#2.2X, f = %#2.2X\r", (int)sc_ACDCU_onoff, (int)unit_bit, (int)f);
if( f > 0 ) {printf("\tALREADY ON"); return;}

sc_ACDCU_onoff |= unit_bit;				//  & save it
pio_pwr_onoff(sc_ACDCU_onoff);
}

//-------- disable all ACDCU
void pwr_disable_all(void)
{
sc_ACDCU_onoff = 0x00;
pio_pwr_onoff(sc_ACDCU_onoff);
}

//-------- disable a particular ACDCU
void pwr_disable_unit(void)
{
int unit_num;
unsigned char unit_bit = 1;
unsigned char f;

unit_num = cd_arg_i();
if(unit_num == -2 ) {prt_needarg(); return;}
if(unit_num < 0 ) return;
if(unit_num > 7 ) {prt_toobig(unit_num); return;}

unit_bit = (unit_bit<<unit_num);		// shift & clear bit in the proper position
f = (sc_ACDCU_onoff & unit_bit);
if(f == 0 ) {printf("\tALREADY OFF"); return;}

sc_ACDCU_onoff &= ~unit_bit;				//  & save it
pio_pwr_onoff(sc_ACDCU_onoff);
}

//-------- output the current ACDCU onoff control byte.
void pwr_onoff(void)
{
printf("\r\tACDCU_onoff\t= %#2.2X\r", (int)sc_ACDCU_onoff);
}

//-------- read and output the ACDCU status byte.
void pwr_status(void)
{
unsigned char pwr_stat;

pwr_stat = pio_pwr_status();
printf("\r\tACDCU_Status = %#2.2X\r", (int)pwr_stat);
}

/*====Unused and test code ============================================================*/

/*
//--------
void ResetOn(void)
{
pio_Reset((bit)ON);
}

//--------
void ResetOff(void)
{
pio_Reset((bit)OFF);
}
*/

/*
//--------
void chk_switches()
{
pio_RdSwitches();
}
*/

/*
void testxx(void )	// assorted code bits & variables for testing
{
	static unsigned char ub;
//	static int i;
//	static unsigned long ul;

	ES = OFF;	// serial interrupts off for speed.

//  pio_Reset((bit)OFF);
//  pio_Reset((bit)ON);
//  pio_Reset((bit)OFF);
//  pio_Reset((bit)ON);
//  pio_nEnable((bit)OFF);
//  pio_nEnable((bit)ON);
//  pio_nEnable((bit)OFF);
//  pio_nEnable((bit)ON);
//
//	pio_Frun_Count(i++);
//	pio_FrameNum(ul++);
//	pio_SyncLength(ul++);
pio_DV_Mode(ub++);
//	pio_pwr_onoff(ub++);
//	ub = pio_pwr_status();

	ES = ON;
//
}
*/

/*===================== EOF ==========================================*/







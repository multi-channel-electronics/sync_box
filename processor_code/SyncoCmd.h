/*==========================================================================*
 * Header file for SyncoCmd 												*
 *==========================================================================*
 * First version: RHJ 1-May-06
 *
 *
 *
 */

#define  OK    		0
#define  ABORT 	   -1

#ifndef  TRUE
#define  TRUE		1
#define  FALSE		0
#endif

#define HIGH      	1
#define LOW     	0
#define ON      	1
#define OFF     	0


#define  RXBUFSIZ	82

/* Parser command table entry structure */
typedef struct
	{
	char *nmem;				/* command mnemonic */
	void  (*fcn)();			/* pointer to associated function */
	char *param;			/* command help, parameter hint string */
	char *help;				/* command help, function string */
	} CMD_ENTRY;



/*============================================================================*/
/*     Function definitions                            */
/*============================================================================*/

#ifndef SIO
extern char sio_rx_data;
extern char sio_rxbuf[];
extern char sio_rx_idx;
extern bit	sio_rx_gotcl;
//
extern char putchar(char c);
extern void sio_Init_9600(void);
//
extern void Timer0_Init(void);
//
//externvoid tst_putchar (void);
//externvoid tst_printf (void);
#endif

#ifndef SYNCOMAIN

extern int sc_row_len ;
extern unsigned char sc_num_row;
extern unsigned char sc_mancho_mode;
//
extern void do_ResetAll(void);
extern void set_Enable(void);
extern void set_Disable(void);
extern void get_Status(void);
extern void set_Row_Len(void);
extern void set_Num_Rows(void);
extern void set_FR_Mode(void);
extern void set_RTS_Mode(void);
extern void set_Frame_Num(void);
extern void set_clk_adj_div(void);
extern void pwr_enable_unit(void);
extern void pwr_disable_unit(void);
extern void pwr_disable_all(void);
extern void pwr_onoff(void);
extern void pwr_status(void);
extern void ResetOn(void);
extern void ResetOff(void);
#endif

#ifndef PIOLIB
extern void pio_Reset(bit b);
extern void pio_nEnable(bit b);
extern void pio_SyncLength(unsigned long sl);
extern void pio_FrameNum(unsigned long fn);
extern void pio_DV_Mode(unsigned char mode);
extern void pio_FRun_Count(int frc);
extern void pio_clk_adj_div(int clk_adj_div);
extern void pio_pwr_onoff(unsigned char enbits);
extern unsigned char pio_pwr_status(void);
//extern void pio_RdSwitches();
//extern void pio_Chk_PSCool();
#endif

#ifndef CMDDICT
extern code CMD_ENTRY cmd_dict[];
extern code int dict_size;
//
extern int cd_tokenize(char *str);
extern int cd_parse(CMD_ENTRY cmd_dict[], int dict_size);
extern int cd_arg_i(void);
extern int cd_arg_ul(unsigned long *ul);
extern void cd_help(void);
#endif


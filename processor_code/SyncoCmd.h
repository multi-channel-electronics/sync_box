/*==========================================================================*
 * Header file for SyncoCmd                                                 *
 *==========================================================================*
 * First version: RHJ 1-May-06
 */


#include <8052.h>

/* The lines below provide some translation from Keil to sdcc directives. */

#define bit __bit
#define code __code

/* Add some special registers particular to this microprocessor. */
__sfr __at(0x9A) BRL;
__sfr __at(0x9B) BDRCON;
__sfr __at(0x8F) CKCON0;
__sfr __at(0xAF) CKCON1;

/* These are pulled out of sdcc's include file mcs51/at89c51ed2.h. */

__sfr __at (0x8F) CKCON0;   //Clock control Register 0
__sfr __at (0xAF) CKCON1;   //Clock control Register 1
__sfr __at (0x9B) BDRCON;       //Baud Rate Control
__sfr __at (0x9A) BRL;      //Baud Rate Reload


static inline void _nop_()
{
    __asm
    nop
    __endasm;
}


#define  OK         0
#define  ABORT     -1

#ifndef  TRUE
#define  TRUE       1
#define  FALSE      0
#endif

#define HIGH        1
#define LOW         0
#define ON          1
#define OFF         0


#define  RXBUFSIZ   82

/* Parser command table entry structure */
typedef struct {
    char *nmem;             /* command mnemonic */
    void  (*fcn)();         /* pointer to associated function */
    char *param;            /* command help, parameter hint string */
    char *help;             /* command help, function string */
} CMD_ENTRY;


/*============================================================================*/
/*     Function definitions                            */
/*============================================================================*/

/* Defined in sio.c */
extern char sio_rxbuf[];
extern char sio_rx_idx;
extern bit  sio_rx_gotcl;

/* Note all interrupt handlers must be visible in main(). */
void serial_IT(void) __interrupt(4);
extern void putchar(char c);
extern void sio_Init_9600(void);
extern void Timer0_Init(void);

/* Defined in SyncoCmd.c */
extern char code version[];

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
extern void do_eep(void);

/* Defined in piolib.c */
extern void pio_Reset(bit b);
extern void pio_nEnable(bit b);
extern void pio_SyncLength(unsigned long sl);
extern void pio_FrameNum(unsigned long fn);
extern void pio_DV_Mode(unsigned char mode);
extern void pio_FRun_Count(int frc);
extern void pio_clk_adj_div(int clk_adj_div);
extern void pio_pwr_onoff(unsigned char enbits);
extern unsigned char pio_pwr_status(void);

/* Defined in cmddict.c */
extern code CMD_ENTRY cmd_dict[];
extern code int dict_size;

extern void cd_tokenize(char *str);
extern void cd_parse(CMD_ENTRY cmd_dict[], int dict_size);
extern int cd_arg_i(void);
extern int cd_arg_ul(unsigned long *ul);
extern void cd_help(void);

/* Defined in strtoul.c -- this was a lib func in Keil. */
unsigned long strtoul(const char *nptr, const char **endptr, register int base);


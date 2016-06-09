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
void putchar(char c);
void sio_Init_9600(void);

/* Defined in SyncoCmd.c */
extern unsigned char code version_num;
extern char code version[];

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
void pwr_enable_unit(void);
void pwr_disable_unit(void);
void pwr_disable_all(void);
void pwr_onoff(void);
void pwr_status(void);
void ResetOn(void);
void ResetOff(void);
void do_Select_Bank(void);

/* Defined in piolib.c */
void pio_Reset(bit b);
void pio_nEnable(bit b);
void pio_SyncLength(unsigned long sl);
void pio_FrameNum(unsigned long fn);
void pio_DV_Mode(unsigned char mode);
void pio_FRun_Count(int frc);
void pio_clk_adj_div(int clk_adj_div);
void pio_pwr_onoff(unsigned char enbits);
unsigned char pio_pwr_status(void);

/* Defined in cmddict.c */
extern code CMD_ENTRY main_cmd_dict[];
extern code CMD_ENTRY eeprom_cmd_dict[];

void cd_tokenize(char *str);
void cd_parse(CMD_ENTRY cmd_dict[]);
int cd_arg_i(void);
int cd_arg_ul(unsigned long *ul);
void cd_help(CMD_ENTRY cmd_dict[]);

/* Defined in strtoul.c -- this was a lib func in Keil. */
unsigned long strtoul(const char *nptr, const char **endptr, register int base);

/* For eeprom.c */

/* Address map for EEPROM -- byte offsets. */
#define EE_ADDR_SYNCO_VER           0x00 /* Version of synco that last
                                            wrote to EEPROM. */
#define EE_ADDR_STATUS              0x01 /* Config byte, bits defined
                                            below as EE_CFG_* */
#define EE_ADDR_SYNC_CFG            0x10 /* Start of bank configurations */

/* The status byte contains flag bits.  The top two bits are used to
   indicate whether the last EEPROM update was successful.  They are
   set to _INCONSISTENT while the EEPROM is being updated, then set to
   _OK when the write is complete.  Lower bits may be used for other
   stuff. */
#define EE_CFG_CHECK_MASK           0xc0
#define EE_CFG_CHECK_INCONSISTENT   0x80
#define EE_CFG_CHECK_OK             0x40
#define EE_CFG_LOAD_ON_STARTUP      0x01  /* If set, causes bank info
                                             to be loaded from EEPROM
                                             when sync box starts up
                                             or is reset. */

char eeprom_get_byte(unsigned char addr);
bit eeprom_set_byte(unsigned char addr, unsigned char value);
unsigned char eeprom_get_bytes(char *dest, unsigned char addr,
                               unsigned char count);
unsigned char eeprom_set_bytes(unsigned char addr, char *source,
                               unsigned char count);

void do_MODE();
void do_help();
void do_EEPROM_mode();
void exit_EEPROM_mode();
void do_EEPROM_dump();
void do_EEPROM_load();
void do_EEPROM_save();
void do_EEPROM_boot_enable();
void do_EEPROM_boot_disable();

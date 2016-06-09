/* -*- mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *      vim: sw=4 ts=4 et tw=80
 */

/*==========================================================================*
 * SyncoCmd.c                                                               *
 * main() and command io routines.                                          *
 *==========================================================================*
 * First Version for SC2: SyncoCmd-V1a RHJ 1-May-06
 * SyncoCmd-V1b RHJ 17-Aug-06   - minor cosmetic changes and additions.
 * 22-Oct-06 RHJ                - added pwr_status() command function.
 * 2010-Feb-25 MA               - adds ckd command to specify a 50MHz divisor
 *                              - (1 to 255) to generate frequencies for
 *                              - DV_Spare1 and DV_Spare2 outputs.
 * 2013-Aug-22 MA               - hard-code Spider values for fr=120,
 *                              - row_len=53, num_rows=33
 * 2015-Nov-05 MFH              - Port source code for sdcc compiler.
 *                              - Start of v30 series.
 * 2016-Jan-20 MFH              - Optional configuration of bank1/bank2.
 *                              - EEPROM load/save, optionally on boot.
 */

#include <stdio.h>
#include <string.h>
#include "SyncoCmd.h"


/*==========================================================================*/

/* Constants for manipulating bits in mancho_mode: */
#define  MODE_FRUN_DV      0x01  /* Otherwise, it's in RTS_DV */
#define  MODE_BANK1        0x02  /* Direct parameters to output Bank 1. */
#define  MODE_BANK2        0x04  /* Direct parameters to output Bank 2. */

/* For settings that are independent of Bank1 vs. Bank2. */
typedef struct {
    unsigned char active_bank;   /* 0 if unbanked,
                                    1/2 for banked with bank1/bank2 active. */
    unsigned char mancho_mode;   /* Carrier for MODE_* bits, see above. */
    unsigned char mancho_enable; /* 1 if sync counters are running. */
    unsigned char ACDCU_onoff;   /* ACDC unit power control bits. */
} syncbox_config_t;

/* The active configuration. */
syncbox_config_t sc;

/* For settings that are specific to Bank1 vs. Bank2. */
typedef struct {
    int row_len;
    unsigned char num_row;
    int FRun_Count;              /* AddrZero count for output of a DV_FreeRun */
    int clk_adj_div;             /* Divisor for the adjustable clock frequency */
} syncbank_config_t;

/* Pointer to the sync box settings that are currently being updated... */
syncbank_config_t *active_config;

/* ... which is one of these. */
syncbank_config_t config_all;      // Settings when targeting both banks.
syncbank_config_t config_bank1;    // Settings specific to bank1.
syncbank_config_t config_bank2;    // Settings specific to bank2.


/* ACT classic defaults */
#define  DEFAULT_FRUN_COUNT     38
#define  DEFAULT_ROWLEN         50
#define  DEFAULT_NUMROW         33
#define  DEFAULT_CLKADJDIV      10

/* Scuba2 classic defaults (~200 Hz DV) */
//#define  DEFAULT_FRUN_COUNT       47
//#define  DEFAULT_ROWLEN           64
//#define  DEFAULT_NUMROW           41


// min-max values for cmd input
#define  MINROWLEN       1  // rowlen setable 50 -> 4095
#define  MAXROWLEN    4095
#define  MINNUMROW       1  // numrow setable 1 -> 63
#define  MAXNUMROW      63
#define  MINSYNCLEN    250  // rowlen * numrow must be > 250
#define  MINFRCNT        1  // DV_FRUN output 1 to 4095 occurances of AddrZero
#define  MAXFRCNT     4095
#define  MINDIV          1  // Minimum 50MHz division ratio for adjustable clk outputs
#define  MAXDIV        255  // Maximum 50MHz division ratio for adjustable clk outputs


/*==========================================================================*/
/*  Note that sdcc does _not_ initialize global non-constant
 *  variables, at least not automatically.  So our globals are either
 *  declared as "code" constants, or are variables that get
 *  initialized in main() or do_ResetAll(). */

/* Code version and its string representation. */
const unsigned char code version_num = 31;
const char code version[] =  "\r\tSyncoCmd-V31\r";

/* Pointer to active command dictionary. */
CMD_ENTRY code *active_cmd_dict;

/* Pointer to active command prompt string... */
char code *active_prompt; 

/* ... which is one of these. */
char code std_prompt[] =    "\rSynco> ";
char code bank1_prompt[] =  "\rSynco [bank 1]> ";
char code bank2_prompt[] =  "\rSynco [bank 2]> ";
char code eeprom_prompt[] = "\rSynco [EEPROM]> ";

/*==========================================================================*/

void set_prompt(char bank, bit EEPROM);
void do_EEPROM_boot_actions();

inline unsigned long get_sync_length(int rl, unsigned char nr)
{
    // Always promote to 32-bit ints before multiply.
    return (unsigned long)rl * (unsigned long)nr;
}

/* ---- set_target_bank

        This function should be called to set up the CPLD to accept
        commands directed to bank 1, bank 2, or both banks (0).  The
        appropriate bits are set in sc.mancho_mode, and the mode is
        written to the CPLD if it has changed.  If sc.mancho_mode has
        not yet been written to CPLD (as on start-up), this function
        should be called with force=TRUE.
*/
void set_target_bank(unsigned char bank, bit force)
{
    /* Write to both banks... */
    unsigned char new_mode = sc.mancho_mode | (MODE_BANK1 | MODE_BANK2);
    if (bank == 1)
        new_mode ^= MODE_BANK2; /* ...except not bank2. */
    else if (bank == 2)
        new_mode ^= MODE_BANK1; /* ...except not bank1. */
    /* Only write if changed.  Though this probably doesn't limit
       performance. */
    if (force || (new_mode != sc.mancho_mode)) {
        sc.mancho_mode = new_mode;
        pio_DV_Mode(sc.mancho_mode);
    }
}

/* ---- send_bank_config

        This writes a whole syncbank_config_t to whatever bank happens
        to be active (as set in mancho_mode).  nEnable should be off
        _before_ you call this routine.  Note that it zeros the
        framenum.
*/
void send_bank_config(syncbank_config_t *cfg)
{
    unsigned long sl = get_sync_length(cfg->row_len, cfg->num_row);
    pio_SyncLength(sl-1);
    pio_FRun_Count(cfg->FRun_Count-1);
    pio_clk_adj_div(cfg->clk_adj_div);
    pio_FrameNum((unsigned long)0);
}

void do_ResetAll(void)
{
    /* Use unbanked mode by default. */
    sc.active_bank = 0;
    active_config = &config_all;

    /* Output will be enabled, but ACDCU off. */
    sc.mancho_enable = TRUE;
    sc.ACDCU_onoff = 0;

    /* Also set FRUN mode. (Comment out to get DV_RTS instead.) */
    sc.mancho_mode |= MODE_FRUN_DV;

    /* Copy defaults into active bank config. */
    active_config->FRun_Count = DEFAULT_FRUN_COUNT;
    active_config->row_len = DEFAULT_ROWLEN;
    active_config->num_row = DEFAULT_NUMROW;
    active_config->clk_adj_div = DEFAULT_CLKADJDIV;

    /* Now disable CPLD operations; reset CPLD logic. */
    pio_nEnable(OFF);
    pio_Reset((bit)ON);
    pio_Reset((bit)OFF);

    /* At this point we update from EEPROM, maybe, which writes the
       new configuration to the CPLD (so must happen after Reset). */
    do_EEPROM_boot_actions();

    /* Re-write current active bank to CPLD. This is enough to cover
       case where EEPROM is not in use; is innocuous otherwise. */
    set_target_bank(sc.active_bank, TRUE);
    send_bank_config(active_config);

    /* After loading all configuration, re-enable. */
    pio_nEnable(sc.mancho_enable);
}

/*==========================================================================*/

void main(void)
{
    sio_Init_9600();
    TI = 1 ;

    printf("\rStarting up...\r\r");

    do_ResetAll();

    active_cmd_dict = main_cmd_dict;
    cd_help(active_cmd_dict);

    set_prompt(sc.active_bank, FALSE);
    printf("%s", active_prompt);
    
    while(1)            // endless
    {
        //testxx();       // test & debug stuff.

        if(sio_rx_gotcl == TRUE) {          // got a cmd line
            sio_rx_gotcl = FALSE;
            ES = OFF;                           // disable serial interrupt
            cd_tokenize(sio_rxbuf);             // parse a command line
            cd_parse(active_cmd_dict);
            sio_rx_idx = 0;                     // reset message pointer
            ES = ON;                            // Enable serial interrupt
            printf("%s", active_prompt) ;
        }
        //chk_switches();
    }
}

/*========== Utility functions for dumping configuration to terminal========*/

void print_syncbox_config(syncbox_config_t *s)
{
    code char fr_yes[] = "FreeRun_DV";
    code char fr_no[]  = "RTS_DV";
    code char on[]  = "ON";
    code char off[] = "OFF";
    char *fr_str = fr_no;
    char *run_str = off;
    
    if (s->mancho_enable)
        run_str = on;
    
    if (sc.mancho_mode & MODE_FRUN_DV)
        fr_str = fr_yes;

    printf("  Mancho_Enable = %s\r", run_str);
    printf("  DV_Mode       = %s\r", fr_str);
    printf("  ACDCU_onoff   = 0x%02x\r", (int)sc.ACDCU_onoff);
}    

void print_syncbank_config(syncbank_config_t *bank) {
    printf("  Num_Row       = %d\r", (int)bank->num_row);
    printf("  Row_Len       = %d\r", bank->row_len);
    printf("  FRun_Count    = %d\r", bank->FRun_Count);
    printf("  Clock_divider = %d\r", bank->clk_adj_div);
}


/*========== Utility Functions for command variable range errors ===========*/

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


/*========== Command Dictionary Functions  =================================*/

//--------
void set_Enable(void)
{
    sc.mancho_enable = TRUE;
    pio_nEnable(ON);
}

//--------
void set_Disable(void)
{
    sc.mancho_enable = FALSE;
    pio_nEnable(OFF);
}

//--------
void get_Status(void)
{
    printf("\r System config:\r");
    print_syncbox_config(&sc);

    if (sc.active_bank == 0) {
        printf(" All-bank configuration:\r");
        print_syncbank_config(&config_all);
    } else {
        printf(" Bank 1 configuration:\r");
        print_syncbank_config(&config_bank1);
        printf(" Bank 2 configuration:\r");
        print_syncbank_config(&config_bank2);
    }
}

//--------
void set_Row_Len(void)
{
    int rl;
    unsigned long sl;

    rl = cd_arg_i();
    if(rl == -2 ) {prt_needarg(); return;}  // if rl = -2 [= no arg], then error msg
    if(rl < 0 ) {return;}                   // if rl = -1 [= not a digit arg], then just ignor
    if(rl < MINROWLEN ) {prt_toosmall(rl); return;}
    if(rl > MAXROWLEN ) {prt_toobig(rl); return;}

    sl = get_sync_length(rl, active_config->num_row);
    if(sl < MINSYNCLEN ) {prt_minsync(); return;}

    active_config->row_len = rl;        // everything ok, set as the new row_len
    pio_nEnable(OFF);                   //
    set_target_bank(sc.active_bank, 0);
    pio_SyncLength(sl - 1);             // minus 1 because counter includes zero
    pio_nEnable(sc.mancho_enable);      //
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

    sl = get_sync_length(active_config->row_len, (unsigned char)nr);
    if(sl < MINSYNCLEN ) {prt_minsync(); return;}

    active_config->num_row = (unsigned char)nr; //
    pio_nEnable(OFF);               //
    set_target_bank(sc.active_bank, 0);
    pio_SyncLength(sl - 1);         // minus 1 because counter includes zero
    pio_nEnable(sc.mancho_enable);  //
}

//--------
void set_FR_Mode(void)
{
    int frc;

    frc = cd_arg_i();
    //printf("\targ = %d\r", frc);
    if (frc >= 0) {
        if (frc < MINFRCNT ) {prt_toosmall(frc); return;}
        if (frc > MAXFRCNT ) {prt_toobig(frc); return;}
        active_config->FRun_Count = frc;
    }

    // if arg = -1 [= not a digit arg], then just ignor
    // if arg = -2 [= no arg], then use previous config.FRUN_COUNT
    if(frc < 0) {
        if(frc == -1)
            return;
        else
            frc = active_config->FRun_Count;
    }

    pio_nEnable(OFF);
    /* Note that v30 and earlier would write 0x00 and 0xFF to DV_Mode.
       But now we give each bit meaning. */
    sc.mancho_mode |= MODE_FRUN_DV;
    /* Force, so MODE_FRUN_DV gets written. */
    set_target_bank(sc.active_bank, TRUE);
    pio_FRun_Count(frc-1);              // minus 1 because counter includes zero
    pio_nEnable(sc.mancho_enable);
}

//-------- switch to RTS_DV mode
void set_RTS_Mode(void)
{
    sc.mancho_mode &= ~MODE_FRUN_DV;
    pio_nEnable(OFF);                   //
    /* Note that v30 and earlier would write 0x00 and 0xFF to DV_Mode.
       But now we give each bit meaning. */
    pio_DV_Mode(sc.mancho_mode);
    pio_nEnable(sc.mancho_enable);      //
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
    pio_nEnable(OFF);               //
    set_target_bank(sc.active_bank, 0);
    pio_FrameNum(fn);
    pio_nEnable(sc.mancho_enable);  //
}

//-------- set the 50MHz/div for DV_SPARE2 output
void set_clk_adj_div(void)
{
    int clk_adj_div_temp;

    clk_adj_div_temp = cd_arg_i();

    if(clk_adj_div_temp < MINDIV ) {prt_toosmall(clk_adj_div_temp); return;}
    if(clk_adj_div_temp > MAXDIV ) {prt_toobig(clk_adj_div_temp); return;}
    active_config->clk_adj_div = clk_adj_div_temp;

    // pio_nEnable(OFF);
    set_target_bank(sc.active_bank, 0);
    pio_clk_adj_div(active_config->clk_adj_div);
    // pio_nEnable(sc.mancho_enable);
}

/* Bank switching management. */

syncbank_config_t *get_syncbank(unsigned char bank)
{
    switch (bank) {
    case 1:
        return &config_bank1;
    case 2:
        return &config_bank2;
    }
    return &config_all;
}

void do_Select_Bank()
{
    unsigned char new_bank;
    int arg = cd_arg_i();
    if (arg < 0 || arg > 2) {
        printf("\tBank argument must be 1 or 2, or 0 for unbanked operation.\r");
        return;
    }
    putchar('\r');
    new_bank = (unsigned char)arg;
    if (new_bank == sc.active_bank)
        return;

    /* Now alert user and perform any necessary CPLD writes. */
    if (new_bank == 0) {
        printf("Switching to unbanked mode.\r");
        printf("Using bank1 configuration for unbanked operation.\r");
        memcpy(&config_all, &config_bank1, sizeof(config_all));
        /* Re-send the settings to make sure bank2 is consistent. */
        pio_nEnable(OFF);
        set_target_bank(0, TRUE);
        send_bank_config(&config_all);
        pio_nEnable(sc.mancho_enable);
    } else if (sc.active_bank == 0) {
        /* We were not banked, but now we will be */
        printf("Switching to banked mode.\r");
        printf("Copying configuration to bank1 and bank2.\r");
        memcpy(&config_bank1, &config_all, sizeof(config_all));
        memcpy(&config_bank2, &config_all, sizeof(config_all));
        /* No writes necessary. */
    }
    /* Repoint active_config to the right place. */
    sc.active_bank = new_bank;
    active_config = get_syncbank(sc.active_bank);

    if (sc.active_bank != 0)
        printf("Bankable commands will now go to bank %i.\r",
               (int)sc.active_bank);

    set_prompt(sc.active_bank, FALSE);
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
    
    unit_bit = (unit_bit<<unit_num);    // shift bit into the proper position
    f = (sc.ACDCU_onoff & unit_bit);
    //printf("\r\tonoff = %#2.2X, ub = %#2.2X, f = %#2.2X\r", (int)sc.ACDCU_onoff, (int)unit_bit, (int)f);
    if( f > 0 ) {printf("\tALREADY ON"); return;}
    
    sc.ACDCU_onoff |= unit_bit;             //  & save it
    pio_pwr_onoff(sc.ACDCU_onoff);
}

//-------- disable all ACDCU
void pwr_disable_all(void)
{
    sc.ACDCU_onoff = 0x00;
    pio_pwr_onoff(sc.ACDCU_onoff);
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

    unit_bit = (unit_bit<<unit_num);        // shift & clear bit in the proper position
    f = (sc.ACDCU_onoff & unit_bit);
    if (f == 0) { printf("\tALREADY OFF"); return;}

    sc.ACDCU_onoff &= ~unit_bit;                //  & save it
    pio_pwr_onoff(sc.ACDCU_onoff);
}

//-------- output the current ACDCU onoff control byte.
void pwr_onoff(void)
{
    printf("\r\tACDCU_onoff\t= 0X%02x\r", (int)sc.ACDCU_onoff);
}

//-------- read and output the ACDCU status byte.
void pwr_status(void)
{
    unsigned char pwr_stat;
    
    pwr_stat = pio_pwr_status();
    printf("\r\tACDCU_Status = 0X%02x\r", (int)pwr_stat);
}


void set_prompt(char bank, bit EEPROM)
{
    if (EEPROM)
        active_prompt = eeprom_prompt;
    else switch(bank) {
        case 1:
            active_prompt = bank1_prompt;
            break;
        case 2:
            active_prompt = bank2_prompt;
            break;
        default:
            active_prompt = std_prompt;
            break;
        }
}

/* EEPROM handling */

void do_EEPROM_mode()
{
    active_cmd_dict = eeprom_cmd_dict;
    set_prompt(0, TRUE);
    do_help();
}
void exit_EEPROM_mode()
{
    active_cmd_dict = main_cmd_dict;
    set_prompt(sc.active_bank, FALSE);
    do_help();
}

void do_help()
{
    cd_help(active_cmd_dict);
}

/* High-level support. */

unsigned char do_EEPROM_read_syncbank(bit inspect_only, char addr,
                                      syncbank_config_t *dest)
{
    syncbank_config_t bank;
    addr = eeprom_get_bytes((char*)&bank, addr, sizeof(bank));
    if (!inspect_only)
        memcpy(dest, &bank, sizeof(bank));
    print_syncbank_config(&bank);
    return addr;
}

/* do_EEPROM_read

   This function reads in the data stored to EEPROM and interprets it.
   Note that the read sequence must correspond to the write sequence
   in do_EEPROM_save.  The exact behaviour of this function is
   determined by the arguments.

   To read and print the info from EEPROM only, pass
      inspect_only=TRUE,  boot_time=FALSE

   To read, print, and load the EEPROM data as the active configuration, pass
      inspect_only=FALSE, boot_time=FALSE
   
   If it is boot-time and you want to load the configuration from EEPROM, pass
      inspect_only=FALSE, boot_time=TRUE
*/

void do_EEPROM_read(bit inspect_only, bit boot_time)
{
    unsigned char eep_status;
    unsigned char src_addr;
    syncbox_config_t sc_copy;
    bit load_on_startup;

    if ((unsigned char)eeprom_get_byte(EE_ADDR_SYNCO_VER) != version_num) {
        if (!boot_time) {
            /* _Quietly_ ignore invalid EEPROM data if this is a load-on-boot attempt. */
            printf("EEPROM data not saved with this version of Synco.\r");
        }
        return;
    }
    eep_status = eeprom_get_byte(EE_ADDR_STATUS);
    switch (eep_status & EE_CFG_CHECK_MASK) {
    case EE_CFG_CHECK_OK:
        break;
    case EE_CFG_CHECK_INCONSISTENT:
        printf("EEPROM data marked as inconsistent (interrupted during write?).\r");
    default:
        printf("EEPROM status byte invalid; refusing to load.\r");
        return;
    }
    
    load_on_startup = eep_status & EE_CFG_LOAD_ON_STARTUP;
    if (boot_time) {
        if (!load_on_startup)
            return;
    } else {
        printf(" EEPROM load on boot = %s", (load_on_startup ?
                                             "enabled" : "disabled"));
    }

    /* Load SC config, then bank configurations. */
    src_addr = EE_ADDR_SYNC_CFG;
    src_addr = eeprom_get_bytes((char*)&sc_copy, src_addr, sizeof(sc));
    if (!inspect_only)
        memcpy(&sc, &sc_copy, sizeof(sc));

    printf("\r System config:\r");
    print_syncbox_config(&sc_copy);

    if (sc_copy.active_bank == 0) {
        printf(" All-bank configuration:\r");
        src_addr = do_EEPROM_read_syncbank(inspect_only, src_addr, &config_all);
    } else {
        printf(" Bank 1 configuration:\r");
        src_addr = do_EEPROM_read_syncbank(inspect_only, src_addr, &config_bank1);
        printf(" Bank 2 configuration:\r");
        src_addr = do_EEPROM_read_syncbank(inspect_only, src_addr, &config_bank2);
    }
    if (inspect_only)
        return;

    /* Now write the config to CPLD.  Pass force=TRUE to
       set_target_bank since this can be run at boot. */
    pio_nEnable(OFF);
    if (sc.active_bank == 0) {
        /* Set to broadcast, write config_all. */
        set_target_bank(0, TRUE);
        send_bank_config(&config_all);
    } else {
        /* Write each bank's info. */
        set_target_bank(1, TRUE);
        send_bank_config(&config_bank1);
        set_target_bank(2, TRUE);
        send_bank_config(&config_bank2);
    }
    printf("Configuration loaded from EEPROM.\r");
    active_config = get_syncbank(sc.active_bank);
    
}

void do_EEPROM_dump()
{
    printf("\rEEPROM contains saved configuration:\r");
    do_EEPROM_read(TRUE, FALSE);
}

void do_EEPROM_load()
{
    putchar('\r');
    do_EEPROM_read(FALSE, FALSE);
}

void do_EEPROM_save()
{
    unsigned char dest_addr;
    /* Invalidate the present data. */
    eeprom_set_byte(EE_ADDR_STATUS, EE_CFG_CHECK_INCONSISTENT);

    /* Check and maybe update the version number. */
    if ((unsigned char)eeprom_get_byte(EE_ADDR_SYNCO_VER) != version_num)
        eeprom_set_byte(EE_ADDR_SYNCO_VER, version_num);
    
    /* Now write the main SC config and bank configurations. */
    dest_addr = EE_ADDR_SYNC_CFG;
    dest_addr = eeprom_set_bytes(dest_addr, (char*)&sc, sizeof(sc));
    if (sc.active_bank == 0) {
        dest_addr = eeprom_set_bytes(dest_addr, (char*)&config_all,
                                     sizeof(config_all));
    } else {
        dest_addr = eeprom_set_bytes(dest_addr, (char*)&config_bank1,
                                     sizeof(config_all));
        dest_addr = eeprom_set_bytes(dest_addr, (char*)&config_bank2,
                                     sizeof(config_all));
    }

    /* Validate the data by writing status OK. */
    eeprom_set_byte(EE_ADDR_STATUS, EE_CFG_CHECK_OK);
}

void do_EEPROM_boot_set(bit on)
{
    unsigned char stat;
    putchar('\r');
    if ((unsigned char)eeprom_get_byte(EE_ADDR_SYNCO_VER) != version_num) {
        printf("EEPROM data not saved with this version of Synco.\r");
        printf("Write valid data first, then manipulate boot mode.\r");
        return;
    }
    stat = eeprom_get_byte(EE_ADDR_STATUS);
    if (on)
        stat |= EE_CFG_LOAD_ON_STARTUP;
    else
        stat &= (0xff ^ EE_CFG_LOAD_ON_STARTUP);
    eeprom_set_byte(EE_ADDR_STATUS, stat);
}

void do_EEPROM_boot_enable()
{
    do_EEPROM_boot_set(ON);
}

void do_EEPROM_boot_disable()
{
    do_EEPROM_boot_set(OFF);
}

void do_EEPROM_boot_actions()
{
    do_EEPROM_read(FALSE, TRUE);
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
void testxx(void )  // assorted code bits & variables for testing
{
    static unsigned char ub;
//  static int i;
//  static unsigned long ul;

    ES = OFF;   // serial interrupts off for speed.

//  pio_Reset((bit)OFF);
//  pio_Reset((bit)ON);
//  pio_Reset((bit)OFF);
//  pio_Reset((bit)ON);
//  pio_nEnable((bit)OFF);
//  pio_nEnable((bit)ON);
//  pio_nEnable((bit)OFF);
//  pio_nEnable((bit)ON);
//
//  pio_Frun_Count(i++);
//  pio_FrameNum(ul++);
//  pio_SyncLength(ul++);
pio_DV_Mode(ub++);
//  pio_pwr_onoff(ub++);
//  ub = pio_pwr_status();

    ES = ON;
//
}
*/

/*========================= EOF ============================================*/


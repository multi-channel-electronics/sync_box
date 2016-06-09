/* -*- mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *      vim: sw=4 ts=4 et tw=80
 */
/*==========================================================================*
 * cmddict.c                                                                *
 * Command dictionary & command parse routines.                             *
 *==========================================================================*
 * First version for SC2: RHJ 1-May-06
 *
 */


/*==========================================================================*/
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "SyncoCmd.h"


/*========== Support Routines Function prototypes   ========================*/
char *nxttoken(void);


// parameter hint strings for help. See cd_help() function definition below.
code char pram[] = " n";
code char nopr[] = "  ";

/*========== command dictionary =============================================*/

code CMD_ENTRY main_cmd_dict[] =
    {
        { "h",      do_help,            nopr,   "help, this stuff" },
        { "?",      get_Status,         nopr,   "get Mancho Status" },
        { "rl",     set_Row_Len,        pram,   "set Row_Len\tn = 1 to 4095" },
        { "nr",     set_Num_Rows,       pram,   "set Num_Rows\tn = 1 to 63" },
        { "rt",     set_RTS_Mode,       nopr,   "set RTS_Mode" },
        { "fr",     set_FR_Mode,        pram,   "set FreeRun Mode\tn = 1 to 4095" },
        { "fn",     set_Frame_Num,      pram,   "set Frame Sequence Number\tn = 0 to 2^32-1" },
        { "ckd",    set_clk_adj_div,    pram,   "set the divisor for the adjustable clock output (50MHz/ckd)\tn = 1 to 255 (default=10)"},
        { "st",     set_Disable,        nopr,   "stop, disable Manchout" },
        { "go",     set_Enable,         nopr,   "enable Manchout" },
        { "dpa",    pwr_disable_all,    nopr,   "pwr disable all" },
        { "dpu",    pwr_disable_unit,   pram,   "pwr disable unit\tn = 0 to 7" },
        { "epu",    pwr_enable_unit,    pram,   "pwr enable unit \tn = 0 to 7" },
        { "pof",    pwr_onoff,          nopr,   "get ACDCU_onoff cntl byte" },
        { "ps",     pwr_status,         nopr,   "get ACDCU status" },
        { "bank",   do_Select_Bank,     pram,   "Change target bank for params" },
        { "eeprom", do_EEPROM_mode,     nopr,   "Enter EEPROM manipulation mode" },
        { "re",     do_ResetAll,        nopr,   "Reset all to defaults" },
//      { "ron",    ResetOn,            nopr,   "Reset on" },
//      { "rof",    ResetOff,           nopr,   "Reset off" },
        { NULL,     NULL,               NULL,   NULL}
    };

code CMD_ENTRY eeprom_cmd_dict[] =
    {
        { "h",            do_help,                nopr,
          "help, this stuff" },
        { "exit",         exit_EEPROM_mode,       nopr,
          "return to main menu" },
        { "dump",         do_EEPROM_dump,         nopr,
          "Print current contents of EEPROM." },
        { "load",         do_EEPROM_load,         nopr,
          "Load configuration from EEPROM." },
        { "save",         do_EEPROM_save,         nopr,
          "Save current configuration to EEPROM." },
        { "enable_boot",  do_EEPROM_boot_enable,  nopr,
          "Set system to load EEPROM parameters on start-up."},
        { "disable_boot", do_EEPROM_boot_disable, nopr,
          "Set system to NOT load EEPROM parameters on start-up."},
        { NULL,     NULL,               NULL,   NULL}
    };

/*========== Parse header definitions =======================================*/

#define MAXTOKEN 12

static char *tokenpbuf[MAXTOKEN];   /* array of pointers to tokens */
static char **tokenv;               /* pointer to current token (pointer to token pointer) */


/*----- chop a command line string into separate word strings (tokens)*/
void cd_tokenize(char *str)
{
    int i=0;                                                    // current token (index)

    tokenpbuf[i++] = strtok(str," \n\r");                       // setup strtok for the scan
    while ((tokenpbuf[i] = strtok(NULL," \n\r")) != NULL) {     // scan the rest of the string
        i++;
        if (i >= (MAXTOKEN-1))                                  // if too many tokens, stop the scan
            break;
    }                  

    tokenpbuf[i] = 00 ;                                         // terminate the list of pointers
    tokenv = &tokenpbuf[0];                                     // set pointer to first token
}


/*----- parse a command line */
void cd_parse(CMD_ENTRY cmd_dict[])
{
    int entry, n;
    char found;
    char *token;

    while((token = nxttoken()) != 0 ) {
        //if (strncmp(token,"#",1)==0) break;       //
        /* look for the current token in the command table */
        found = 0;
        for (entry=0; cmd_dict[entry].nmem!=NULL; entry++ ) {
            n = strlen(cmd_dict[entry].nmem);
            // if token is found, execute cmd function
            if(strncmp(token, cmd_dict[entry].nmem, n)==0) {
                found = 1;
                (cmd_dict[entry].fcn)();
                break;
            }
        }
        if (!found) {
            printf("\tWHAT? \"%s\"", token);    // else send error message,
            break;                              //and stop parsing
        }
    }
}


/*-----  supply pointer to token or 00, & maybe increment the token vector */
char *nxttoken()
{
    extern char **tokenv;

    if(*tokenv != 0)
        return(*tokenv++);
    else
        return(0);
}


/*----- return an integer arg from the token list */
int cd_arg_i()
{
    char *nxtarg = nxttoken();

    if (nxtarg == NULL )
        return(-2);                 // if rtn = -2 [= no arg]

    if (isdigit(*nxtarg) == 0) {
        printf("\tWHAT? \"%s\"", nxtarg);
        return(-1);                 // if rtn = -1 [= not a digit arg]
    }

    return(atoi(nxtarg));
}


/* return an unsigned long arg from the token list */
int cd_arg_ul(unsigned long *ul)
{
    char *nxtarg = nxttoken();
    char *p;

    if (nxtarg == NULL)
        { return(-2); }

    if (isdigit(*nxtarg) == 0)
        {
        printf("\tWHAT? \"%s\"", nxtarg);
        return(-1);
        }

    *ul = strtoul(nxtarg, &p, 10);
    return(1);
}

char putstr(char *c)
{
    char i = 0;
    while (*c != 0) {
        putchar(*(c++));
        i++;
    }
    return i;
}

/*----- ouput the command table nmemonics and help strings */
void cd_help(CMD_ENTRY cmd_dict[])
{
    int entry;
    const code char preamble[] = "\r\t ";
    for (entry=0; cmd_dict[entry].nmem!=NULL; entry++) {
        char field_width = 16;
        putstr(preamble);
        field_width -= putstr(cmd_dict[entry].nmem);
        field_width -= putstr(cmd_dict[entry].param);
        while (field_width > 0) {
            putchar('\t');
            field_width -= 8;
        }
        putstr(cmd_dict[entry].help);
    }
}


/*==================== End Of File ======================================== */

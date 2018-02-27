/* -*- mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *      vim: sw=4 ts=4 et tw=80
 */
#include <stdio.h>
#include <stdlib.h>
#include "SyncoCmd.h"

/* EEPROM access

   Since this requires inline assembly, it's dangerous and awkward.
   We use global variables to pass address and data into the assembly
   routines, since the names for locally scoped variables do not seem
   to be predictable.  Note that setting EECON = EEE means that data
   are read/written from EEPROM "latches" instead of "external
   memory".  But since our C variables are all in external memory, you
   can't access them as long as EECON = EE. This is why we load the
   eeaddr and eedata in registers before setting EE.
*/

__sfr __at (0xD2) EECON;    //EEPROM Data Control
#define EEE    0x02             //EEPROM Enable. '1'=use EEPROM, '0'=use XRAM
#define EEBUSY 0x01             //EEPROM Busy. '1'=EEPROM is busy programming

__sfr __at (0x8E) AUXR;         //Auxiliary function register
#define DPU  0x80               //'1'=Disables weak pull-up
#define M0   0x20               //'1'=Strechs MOVX control signals

/* Local prototypes */
void ee_read_asm();
void ee_write_asm();
char eeaddr;
char eedata;

/* The public interface... read/write to some address. */

unsigned char eeprom_get_bytes(char *dest, unsigned char addr,
                               unsigned char count)
{
    while (count-- > 0)
        *(dest++) = eeprom_get_byte(addr++);
    return addr;
}

unsigned char eeprom_set_bytes(unsigned char addr, char *source,
                               unsigned char count)
{
    while (count-- > 0)
        eeprom_set_byte(addr++, *(source++));
    return addr;
}
 
char eeprom_get_byte(unsigned char addr)
{
    char auxr_cache = AUXR;
    eeaddr = addr;
    eedata = 8;

    AUXR |= M0;
    ee_read_asm();
    AUXR = auxr_cache;
    return eedata;
}

bit eeprom_set_byte(unsigned char addr, unsigned char value)
{
    unsigned int counter = 0;
    eeaddr = addr;
    eedata = value;

    ee_write_asm();

    /* Measurement: when working this loop exits with counter=7800,
       and the write rate is about 20 bytes per second. */
    //while (EECON & EEBUSY)
    //    counter++;
    //printf("... eeprom burn took %i loop iters.\r");

    /* Time out after 60k writes. */
    while (counter++ < 60000) {
        if (!(EECON & EEBUSY))
            return TRUE;
    }

    printf("Warning! Timed-out waiting for EEBUSY to drop. EEPROM write may have failed.\r");
    return FALSE;
}


void ee_read_asm()
{
    /* Read value from EEPROM, store in eedata. */
    __asm__(
            /* Save things */
            "        push DPH                                                \n"
            "        push DPL                                                \n"
            "        push A                                                  \n"
            /* Put the address into A for a minute */
            "        mov DPTR,#_eeaddr                                       \n"
            "        movx A,@DPTR                                            \n"
            /* Go to EEPROM access */
            "        mov _EECON,#0x02                                        \n"
            "        mov DPL,A                                               \n"
            "        clr A                                                   \n"
            "        mov DPH,A                                               \n"
            "        movx A,@DPTR                                            \n"
            /* Come back from EEPROM access */
            "        mov _EECON,#0x00                                        \n"
            "        mov DPTR,#_eedata                                       \n"
            "        movx @DPTR,A                                            \n"
            /* Restore things */
            "        pop  A                                                  \n"
            "        pop  DPL                                                \n"
            "        pop  DPH                                                \n"
            );
}

void ee_write_asm()
{
    /* Write value, stored in eedata, to EEPROM[eeaddr]. */
    __asm__(
            /* Save things */
            "        push DPH                                                \n"
            "        push DPL                                                \n"
            "        push A                                                  \n"
            "        push B                                                  \n"
            /* Put the data into B  and the address into A for a minute */
            "        mov DPTR,#_eedata                                       \n"
            "        movx A,@DPTR                                            \n"
            "        mov B,A                                                 \n"
            "        mov DPTR,#_eeaddr                                       \n"
            "        movx A,@DPTR                                            \n"
            /* Go to EEPROM access */
            "        mov _EECON,#0x02                                        \n"
            "        mov DPL,A                                               \n"
            "        clr A                                                   \n"
            "        mov DPH,A                                               \n"
            "        mov A,B                                                 \n"
            "        movx @DPTR,A                                            \n"
            /* Start the burn */
            "        mov _EECON,#0x52                                        \n"
            "        mov _EECON,#0xA2                                        \n"
            /* Restore movx */
            "        mov _EECON,#0x00                                        \n"
            "        pop  B                                                  \n"
            "        pop  A                                                  \n"
            "        pop  DPL                                                \n"
            "        pop  DPH                                                \n"
            );
}

void do_eep(void)
{
    unsigned char rl;
    unsigned char adr;
    rl = cd_arg_i();
    adr = rl & 0xf;
    if (rl != adr)
        eeprom_set_byte(adr, 0x74);
    else {
        char x = eeprom_get_byte(adr);
        printf("  Read %x = %x\r", (int)adr, (int)x);
    }
}

/*==========================================================================* 
 * cmddict.c															* 
 * Command dictionary & command parse routines. 							* 
 *==========================================================================* 
 * First version for SC2: RHJ 1-May-06
 *
 */


/*==========================================================================*/
#define CMDDICT

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "SyncoCmd.h"


/*========== Support Routines Function prototypes   ========================*/
 
int cd_tokenize(char *str);
int cd_parse(CMD_ENTRY cmd_dict[], int dict_size);
char *nxttoken(void);
int cd_arg_i(void);
int cd_arg_ul(unsigned long *ul);
void cd_help();

//code char *prtWHAT[] = "\tWHAT? \"%s\"";

// parameter hint strings for help. See cd_help() function definition below.
code char pram[] = " n";	
code char nopr[] = "  ";

/*========== command dictionary =============================================*/

code CMD_ENTRY cmd_dict[] =
	{
		{ "h", 		cd_help,			nopr,	"help, this stuff" },
		{ "?", 		get_Status,			nopr,	"get Mancho Status" },
		{ "rl", 	set_Row_Len,		pram,	"set Row_Len\tn = 1 to 4095" },
		{ "nr",	 	set_Num_Rows,		pram,	"set Num_Rows\tn = 1 to 63" }, 
		{ "rt", 	set_RTS_Mode,		nopr,	"set RTS_Mode" },
		{ "fr", 	set_FR_Mode,		pram,	"set FreeRun Mode\tn = 1 to 4095" },
		{ "fn", 	set_Frame_Num,		pram,	"set Frame Sequence Number\tn = 0 to 2^32-1" },
		{ "st", 	set_Disable,		nopr,	"stop, disable Manchout" },
		{ "go", 	set_Enable,			nopr,	"enable Manchout" },
		{ "dpa", 	pwr_disable_all,	nopr,	"pwr disable all" },
		{ "dpu", 	pwr_disable_unit,	pram,	"pwr disable unit\tn = 0 to 7" },
		{ "epu", 	pwr_enable_unit,	pram,	"pwr enable unit \tn = 0 to 7" },
		{ "pof", 	pwr_onoff,			nopr,	"get ACDCU_onoff cntl byte" },
		{ "ps", 	pwr_status,			nopr,	"get ACDCU status" },
		{ "re", 	do_ResetAll,		nopr,	"Reset all to defaults" },
//		{ "ron", 	ResetOn,			nopr,	"Reset on" },
//		{ "rof", 	ResetOff,			nopr,	"Reset off" },
	};	

code int dict_size = (sizeof(cmd_dict) / sizeof(CMD_ENTRY));


/*========== Parse header definitions =======================================*/

#define MAXTOKEN 12

static char *tokenpbuf[MAXTOKEN];	/* array of pointers to tokens */
static char **tokenv;				/* pointer to current token (pointer to token pointer) */


/*----- chop a command line string into separate word strings (tokens)*/
int
cd_tokenize(char *str)
{
	extern char *tokenpbuf[];
	extern char **tokenv;
	int i=0;													// current token (index)

	tokenpbuf[i++] = strtok(str," \n\r");						// setup strtok for the scan
	while ((tokenpbuf[i] = strtok(NULL," \n\r")) != NULL)		// scan the rest of the string
		{ i++; if(i >= (MAXTOKEN-1))  break; }					// if too many tokens, stop the scan
		
	tokenpbuf[i] = 00 ;											// terminate the list of pointers
	tokenv = &tokenpbuf[0];										// set pointer to first token
}


/*----- parse a command line */
int 
cd_parse(CMD_ENTRY cmd_dict[], int dict_size)
{
	int entry=0, n;
	char *token;

	while((token = nxttoken()) != 0 ) {
		//if (strncmp(token,"#",1)==0) break;		// 

		/* look for the current token in the command table */
		for (entry=0; entry < dict_size; entry++ ) {
			n = strlen(cmd_dict[entry].nmem);
			if(strncmp(token, cmd_dict[entry].nmem, n)==0) break;	// if token is found, stop looking  
		}

		
		if (entry < dict_size) 
			(cmd_dict[entry].fcn)();			// if token found, execute cmd function	
		else {
			printf("\tWHAT? \"%s\"", token);	// else send error message, 
			break;								//and stop parsing
		}
	}
}


/*-----  supply pointer to token or 00, & maybe increment the token vector */
char *
nxttoken() 
{
	extern char **tokenv;

	if(*tokenv != 0) 
		return(*tokenv++);
	else
		return(0);
}


/*----- return an integer arg from the token list */
int
cd_arg_i()
{
	char *nxtarg = nxttoken();

	if (nxtarg == NULL )
		{ return(-2); }				// if rtn = -2 [= no arg] 
		
	if (isdigit(*nxtarg) == 0)
		{ 
		printf("\tWHAT? \"%s\"", nxtarg);
		return(-1);					// if rtn = -1 [= not a digit arg]
		}
		
	return(atoi(nxtarg));
}


/* return an unsigned long arg from the token list */ 
int 
cd_arg_ul(unsigned long *ul)
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

/*----- ouput the command table nmemonics and help strings */
void
cd_help()
{
	int entry;

	for (entry=0; entry < dict_size; entry++ ) 
		{
		printf("\r\t %s%s\t\t%s", cmd_dict[entry].nmem, cmd_dict[entry].param, cmd_dict[entry].help );
		}
}



/*==================== End Of File ======================================== */

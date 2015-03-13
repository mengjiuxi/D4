#ifndef FILE_FUNCTION_H
#define FILE_FUNCTION_H

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include "uart.h"
#include "xitoa.h"
#include "ff.h"
#include "diskio.h"
#include "tft.h"
#include <string.h>

char fPath[100];
BYTE Buff[1024];			/* Working buffer */
char str;
class tft_t tft;

DWORD AccSize;				/* Work register for fs command */
WORD AccFiles, AccDirs;
FILINFO Finfo;
BYTE RtcOk;					/* RTC is available */
char Line[100];				/* Console input buffer */

FATFS Fatfs[_VOLUMES];		/* File system object for each logical drive */
FIL File[2];				/* File object */


void init_tft(void);
static void ioinit (void);
static void put_rc (FRESULT rc);
int readfile(void);
void disk_init(void);
void file_init(void);
void file_open(void);
void file_write(char *str,BYTE *Buff);
void file_read(BYTE *Buff);
void file_list(void);

#endif


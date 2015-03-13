#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <string.h>
#include "uart.h"
#include "xitoa.h"
#include "ff.h"
#include "diskio.h"
#include "tft.h"
#include <string.h>
class tft_t tft;

void init(void)
{
    //DDRB |= 0x80;			// LED
    //PORTB |= 0x80;
    tft.init();
    tft.setOrient(tft.FlipLandscape);
    tft.setBackground(0x0000);
    tft.setForeground(0x667F);
    tft.setZoom(2);
    tft.clean();
    stdout = tftout(&tft);
    tft.setBGLight(true);
    xdev_out(tftput);
}



DWORD AccSize;				/* Work register for fs command */
WORD AccFiles, AccDirs;
FILINFO Finfo;
BYTE RtcOk;					/* RTC is available */
char Line[100];				/* Console input buffer */

FATFS Fatfs[_VOLUMES];		/* File system object for each logical drive */
FIL File[2];				/* File object */

char fPath[100];
BYTE Buff[1024];			/* Working buffer */

volatile UINT Timer;		/* Performance timer (100Hz increment) */


static
void ioinit (void)
{

    PORTB = 0b11110011;


    //initialise timer 0 to interrupt every 10 ms
    TIMSK0 |= (1 << OCIE0A);
    TCCR0A |= (1 << WGM01);
    OCR0A = 117; //10ms interrupt at 12MHz
    TCCR0B |= (1 << CS02) | (1 << CS00);

}
static
void put_rc (FRESULT rc)
{
    const char *p;
    static const char PROGMEM str[] =
            "OK\0" "DISK_ERR\0" "INT_ERR\0" "NOT_READY\0" "NO_FILE\0" "NO_PATH\0"
            "INVALID_NAME\0" "DENIED\0" "EXIST\0" "INVALID_OBJECT\0" "WRITE_PROTECTED\0"
            "INVALID_DRIVE\0" "NOT_ENABLED\0" "NO_FILE_SYSTEM\0" "MKFS_ABORTED\0" "TIMEOUT\0"
            "LOCKED\0" "NOT_ENOUGH_CORE\0" "TOO_MANY_OPEN_FILES\0";
    uint8_t i;

    for (p = str, i = 0; i != rc && pgm_read_byte_near(p); i++) {
        while(pgm_read_byte_near(p++));
    }
    xprintf(PSTR("rc=%u FR_%S\n"), rc, p);
}

static
FRESULT scan_files (
        char* path		/* Pointer to the working buffer with start path */
        )
{
    DIR dirs;
    FRESULT res;
    int i;
    char *fn;

    res = f_opendir(&dirs, path);
    if (res == FR_OK) {
        i = strlen(path);
        while (((res = f_readdir(&dirs, &Finfo)) == FR_OK) && Finfo.fname[0]) {
            if (_FS_RPATH && Finfo.fname[0] == '.') continue;
            if (Finfo.fattrib & AM_DIR) {
                AccDirs++;
                *(path+i) = '/'; strcpy(path+i+1, fn);
                res = scan_files(path);
                *(path+i) = '\0';
                if (res != FR_OK) break;
            } else {
                //				xprintf(PSTR("%s/%s\n"), path, fn);
                AccFiles++;
                AccSize += Finfo.fsize;
            }
        }
    }

    return res;
}

int main(void)
{	
    ioinit();
    sei();
    init();

    char *ptr, *ptr2;
    long p1, p2, p3;
    FRESULT res;
    BYTE b1, *bp;
    UINT s1, s2, cnt;
    DWORD ofs, sect = 0;
    FATFS *fs;
    DIR dir;
    //RTC rtc;

    tft.clean();
    tft.setZoom(1);

    //di:disk initialisation
    xprintf(PSTR("rc=%d "), disk_initialize(0));
    if(!disk_initialize(0))
        printf("Disk_initialisation successed\n");


    //fi: file initialisation
    put_rc(f_mount(0, &Fatfs[0]));
    if (!f_mount(0,&Fatfs[0]))
        printf("file initialisation successed\n");


    //fo
    //for(long int i=1;;i++)
    //
    uint16_t	 i=0;
    char string[]="I'm not a DOUBI!\n";
    sprintf(fPath,"WHAT%d.TXT", i);//printf file name into fPath
    put_rc(f_open(&File[0],fPath, 0x0A));
    //put_rc(f_open(&File[0], fPath, 6));
    //mode 1: open and read
    //mode 6: open and write

    //fw <len> <val> - write file
    strcpy ( (char *)Buff, string );
    p2 = 0;
    cli(); Timer = 0; sei();
    p1=sizeof(string);
    while (p1) {
        if (p1 >= sizeof Buff)	{ cnt = sizeof Buff; p1 -= sizeof Buff; }
        else 			{ cnt = (WORD)p1; p1 = 0; }
        res = f_write(&File[0], Buff, cnt, &s2);
        if (res != FR_OK) { put_rc(res); break; }
        p2 += s2;
        if (cnt != s2) break;
    }
    cli(); s2 = Timer; sei();
    xprintf(PSTR("%lu bytes written with %lu bytes/sec.\n"), p2, s2 ? (p2 * 100 / s2) : 0);
    f_close(&File[0]);
    put_rc(f_open(&File[0],fPath, 1));
    i++;
    //}


    //fr <len> - read file
    p2 = 0;
    cli(); Timer = 0; sei();
    while ( !f_eof(&File[0])) {
        //if (p1 >= sizeof Buff)	{ cnt = sizeof Buff; p1 -= sizeof Buff; }
        //else 			{ cnt = (WORD)p1; p1 = 0; }
        res = f_read(&File[0], Buff, 1, &s2);
        if (res != FR_OK) { put_rc((FRESULT)res); break; }
        //p2 += s2;
        //if (cnt != s2) break;
        //OCR2B = *Buff;
        //while (!(TIFR1 & _BV(OCF1A)));
        //TIFR1 |= _BV(OCF1A);
        putchar(*Buff);

    }

    /* fl [<path>] - Directory listing */
    res = f_opendir(&dir, ".");//.means current directory
    if (res) { put_rc(res);}
    p1 = s1 = s2 = 0;
    for(;;) {
        res = f_readdir(&dir, &Finfo);
        puts((const char*)Finfo.fname);
        putchar('\n');
        if ((res != FR_OK) || !Finfo.fname[0]) break;/* Break on error or end of dir */
        if (Finfo.fattrib & AM_DIR) {
            s2++;
        } else {
            s1++; p1 += Finfo.fsize;
        }
    }
    xprintf(PSTR("%4u File(s),%10lu bytes total\n%4u Dir(s)"), s1, p1, s2);
    if (f_getfree(ptr, (DWORD*)&p1, &fs) == FR_OK)
        xprintf(PSTR(", %10luK bytes free\n"), p1 * fs->csize / 2);

    //cli(); s2 = Timer; sei();
    xprintf(PSTR("%lu bytes read with %lu bytes/sec.\n"), p2, s2 ? (p2 * 100 / s2) : 0);

    return 1;
}

ISR(TIMER0_COMPA_vect)
{
    Timer++;			/* Performance counter for this module */
    disk_timerproc();	/* Drive timer procedure of low level disk I/O module */
}




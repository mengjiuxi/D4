#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <string.h>
#include "uart.h"
#include "xitoa.h"
#include "ff.h"
#include "diskio.h"
#include "tft.h"

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
#if _USE_LFN
char Lfname[_MAX_LFN+1];
#endif

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
    DDRD  = 0b01000000;
    PORTD = 0b01000000;


    //initialise timer 0 to interrupt every 10 ms
    TIMSK0 |= (1 << OCIE0A);
    TCCR0A |= (1 << WGM01);
    OCR0A = 117; //10ms interrupt at 12MHz
    TCCR0B |= (1 << CS02) | (1 << CS00);
    TCCR1A = 0;
    TCCR1B = (1 <<WGM12) | _BV(CS10);//initialise timer1 with CTC mode
    TCCR2A = (1 <<WGM20);//waveform genration mode, PWM phase correct mode
    TCCR2A |=(1 << COM2B1);//fast pwm mode
    TCCR2B = _BV(CS20);//no prescalar
    OCR1A = 1500;//output frequency 12000000/OCR1A
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
    //printf("rc=%d\n", disk_initialize(0));

    //fi: file initialisation
    put_rc(f_mount(0, &Fatfs[0]));
    if (!f_mount(0,&Fatfs[0]))
        printf("file initialisation successed\n");


    //fo file_open
    f_open(&File[0], "nyan.wav", 1);

    //mode 1: open and read
    //mode 6: open and write

    //fr <len> - read file
    p2 = 0;
    cli(); Timer = 0; sei();
    // read whole file
    while ( !f_eof(&File[0])) {
        //if (p1 >= sizeof Buff)	{ cnt = sizeof Buff; p1 -= sizeof Buff; }
        //else 			{ cnt = (WORD)p1; p1 = 0; }
        res = f_read(&File[0], Buff, 1, &s2);
        if (res != FR_OK) { put_rc((FRESULT)res); break; }
        //p2 += s2;
        //if (cnt != s2) break;
        OCR2B = *Buff;
        while (!(TIFR1 & _BV(OCF1A)));//8000Hz input rate
        TIFR1 |= _BV(OCF1A);//clear the flag
        //putchar(*Buff);
    }
    //cli(); s2 = Timer; sei();
    xprintf(PSTR("%lu bytes read with %lu bytes/sec.\n"), p2, s2 ? (p2 * 100 / s2) : 0);

    //puts((char *)Buff);
    return 1;
}

ISR(TIMER0_COMPA_vect)
{
    Timer++;			/* Performance counter for this module */
    disk_timerproc();	/* Drive timer procedure of low level disk I/O module */
}



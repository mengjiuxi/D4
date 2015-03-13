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
#include "file_function.h"


//BYTE Buff[1024];			/* Working buffer */

volatile UINT Timer;		/* Performance timer (100Hz increment) */


void init_tft(void)
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


void ioinit (void)
{
        PORTB = 0b11110011;
        //initialise timer 0 to interrupt every 10 ms
        TIMSK0 |= (1 << OCIE0A);
        TCCR0A |= (1 << WGM01);
        OCR0A = 117; //10ms interrupt at 12MHz
        TCCR0B |= (1 << CS02) | (1 << CS00);
}

static void put_rc (FRESULT rc)
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

void disk_init(void){
	xprintf(PSTR("rc=%d "), disk_initialize(0));
	if(!disk_initialize(0))
	printf("Disk_initialisation successed\n");
}

void file_init(void){
	put_rc(f_mount(0, &Fatfs[0]));
	if (!f_mount(0,&Fatfs[0]))
	printf("file initialisation successed\n");

}

void file_open(){
	uint16_t i=0;	//make a global i to count the file number
	char fPath[100];	
	sprintf(fPath,"WHAT%d.TXT", i);//printf file name into fPath
	put_rc(f_open(&File[0],fPath, 0x0A));//create a new file
	//put_rc(f_open(&File[0], fPath, 6));
	//mode 1: open and read
	//mode 6: open and write
}

void file_write(char *str, BYTE *Buff){
	int i=0;	
	FRESULT res;
	UINT s2, cnt;	
	long p1, p2;
	strcpy ( (char *)Buff, str );
	p2 = 0;
	p1=sizeof(str);
	while (p1) {
		if (p1 >= sizeof Buff)	{ cnt = sizeof Buff; p1 -= sizeof Buff; }
		else 			{ cnt = (WORD)p1; p1 = 0; }
		res = f_write(&File[0], Buff, cnt, &s2);
		if (res != FR_OK) { put_rc(res); break; }
		p2 += s2;
		if (cnt != s2) break;
	}
	xprintf(PSTR("%lu bytes written with %lu bytes/sec.\n"), p2, s2 ? (p2 * 100 / s2) : 0);
	f_close(&File[0]);//close the file after read.
	put_rc(f_open(&File[0],fPath, 1));
	i++;//Be global

}

//fr <len> - read file

void file_read(BYTE *Buff){
	FRESULT res;
	UINT s2;	
	
	while ( !f_eof(&File[0])) {        
	res = f_read(&File[0], Buff, 1, &s2);
        if (res != FR_OK) { put_rc((FRESULT)res); break; }
        putchar(*Buff);
	
	}
}

//fl [<path>] - Directory listing 
void file_list(){
	char *ptr;	
	FRESULT res;
	UINT s1, s2;	
	long p1, p2;
	FATFS *fs;	
	DIR dir;
	res = f_opendir(&dir, ".");//.means current directory
	if (res) { put_rc(res);}
	p1 = s1 = s2 = 0;
	for(;;) {
		res = f_readdir(&dir, &Finfo);
		puts((const char*)Finfo.fname);
		putchar('\n');
		if ((res != FR_OK) || !Finfo.fname[0]) break;// Break on error or end of dir 
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

}


int main(void)
{	
	//char *ptr, *ptr2;
	//long p1, p2, p3;
	//FRESULT res;	
	//BYTE b1, *bp;
	//UINT s1, s2, cnt;
	//DWORD ofs, sect = 0;
	//FATFS *fs;
	//DIR dir;
	//RTC rtc;
	char str[]={"TEAMARMAGEDDON\n"};	
	BYTE Buff[1024];	
	ioinit();
        sei();
	init_tft();
        tft.clean();
        tft.setZoom(1);
        
	disk_init();
	file_init();
	file_open();
	file_write(str, Buff);
	file_read(Buff);
	file_list();

return 1;
}

ISR(TIMER0_COMPA_vect)
{
	Timer++;			/* Performance counter for this module */
	disk_timerproc();	/* Drive timer procedure of low level disk I/O module */
}


#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <util/delay.h>
#include <tft.h>
#include <rtouch.h>
#include <eemem.h>
#include <adc.h>

//#define AUTO_COLOUR

tft_t tft;
rTouch touch(&tft);

void init(void)
{
	DDRB |= 0x80;			// LED
	PORTB |= 0x80;
	adc_init();
	adc_enable();
	tft.init();
	tft.setOrient(tft.Portrait);
	tft.setBackground(0x0000);
	tft.setForeground(0x667F);
	stdout = tftout(&tft);
	touch.init();
	sei();

	tft.setBGLight(true);
	touch.calibrate();
	tft.clean();
	eeprom_first_done();
}

int main(void)
{
	init();
	tft.clean();
	tft.setZoom(1);
	puts("*** Touch ***");

loop:
	if (touch.pressed()) {
		rTouch::coord_t res = touch.position();
		printf("%d/%d, ",res.x,res.y);	
	}
	goto loop;

	return 1;
}

#include <avr/io.h>

uint8_t GetKey()
{
    uint8_t r,c;

    PORTB|= 0X0F;//set 00001111 high Z

    for(c=0;c<3;c++)
    {
        DDRB&=~(0X7F);// set PB0 to PB6 as input
        DDRB|=(0X40>>c);//select which pin to be high
        for(r=0;r<4;r++)
        {
            if(!(PINB & (0X08>>r)))
            {
                return (3*r+c);
            }
        }
    }

    return 0XFF;//no input
}


void main()
{

    uint8_t key;

    while(1)
    {
        key=GetKey(); //Get the keycode of pressed key

    }

}


#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <util/delay.h>
#include <tft.h>
#include <rgbconv.h>
#include <rtouch.h>
#include <inttypes.h>
#include <eemem.h>
#include <adc.h>


class tft_t tft; //Class for display library
rTouch touch(&tft);

#define color1 0x000F 
#define color2 0x03E0
#define color3 0x780F
#define color4 0xF800      
#define highlight 0xFFFF

/*void init_uart(void)
{
        const int baud_rate =9600;
        UBRROH = (F_CPU/(baud*rate*16L)-1)>>8;
        UBRROL = (F_CPU/(baud*rate*16L)-1);
        UCSROB = _BV(REXN0) | _BV(TEXN0);
        UCSROC = _BV(UCSZ00) | _BV(UCSZ01);
}*/

char get_ch(void);
void put_char(char ch);
void init(void);
int getKey();
int confirm_send();
void menu_background(void);
void send_message(void);
void text_menu(void);
void text_input(void);
void menu(void);
void panic(void);
void voice(void);



char get_ch(void)
{
    while(!(UCSR0A & _BV(RXC0)));
    return UDR0;
}

void put_char(char ch)
{
    while(!(UCSR0A & _BV(UDRE0)));
    UDR0=ch;
}


void init(void) //Init function for display
{
    DDRB |= 0x80;			// LED
    PORTB |= 0x80;
    adc_init();
    adc_enable();
    tft.init();
    tft.setOrient(tft.FlipLandscape);
    tft.setBackground(conv::c32to16(0x000000));
    tft.setForeground(0xFFFF);
    stdout = tftout(&tft);
    touch.init();
    sei();

    tft.setBGLight(true);
    touch.calibrate();
    tft.clean();
    eeprom_first_done();
}

int getKey()  //Function to get the key pressed
{
    DDRB &= ~_BV(0); //X1
    DDRB &= ~_BV(1); //X2
    DDRB &= ~_BV(2); //X3

    DDRB |= _BV(3); //Y1
    DDRB |= _BV(4); //Y2
    DDRB |= _BV(5); //Y3
    DDRB |= _BV(6); //Y4

    PORTB &= ~_BV(3); //Set Ports Low
    PORTB &= ~_BV(4);
    PORTB &= ~_BV(5);
    PORTB &= ~_BV(6);



    PORTB |= _BV(3); //Set Port High
    _delay_ms(5);		//Give it a delay for the voltage to set into place
    if(PINB & _BV(0)){ //Check X1, X2 and X3
        return 1;		//This is repeated for Y1,Y2,Y3,Y4 and the pressed button is returned
    }
    else if(PINB & _BV(1)){
        return 2;
    }
    else if(PINB & _BV(2)){
        return 3;
    }

    PORTB &= ~_BV(3); //Set Port Low

    PORTB |= _BV(4); //Set Port High
    _delay_ms(5);

    if(PINB & _BV(0)){
        return 4;
    }
    else if(PINB & _BV(1)){
        return 5;
    }
    else if(PINB & _BV(2)){
        return 6;
    }

    PORTB &= ~_BV(4); //Set Port Low

    PORTB |= _BV(5); //Set Port High
    _delay_ms(5);

    if(PINB & _BV(0)){
        return 7;
    }
    else if(PINB & _BV(1)){
        return 8;
    }
    else if(PINB & _BV(2)){
        return 9;
    }

    PORTB &= ~_BV(5); //Set Port Low

    PORTB |= _BV(6); //Set Port High
    _delay_ms(5);

    if(PINB & _BV(0)){
        return 10;
    }
    else if(PINB & _BV(1)){
        return 11;
    }
    else if(PINB & _BV(2)){
        return 12;
    }

    PORTB &= ~_BV(6); //Set Port Low

    return 100; //the value 100 is returned if no button is pressed

}

int confirm_send(){

    tft.setForeground(0xFFFF);
    tft.setZoom(3);
    tft.clean();

    tft.rectangle(0, 0, 320, 120, 0x0000);
    tft.setBackground(0x0000);
    tft.setXY(45,50);
    puts("Send Message?");

    tft.rectangle(0, 120, 160, 120, color1);
    tft.setBackground(color1);
    tft.setXY(40,170);
    puts("Send");

    tft.rectangle(160, 120, 160, 120, color3);
    tft.setBackground(color3);
    tft.setXY(185,170);
    puts("Cancel");

    int key;
    int position = 0;

    while(1)
    {

        key=getKey();


        if(position==0){
            tft.frame(0, 120, 160, 121,5, highlight);
            tft.frame(160, 120, 160, 121,5, color3);

        }
        else if(position==1){
            tft.frame(0, 120, 160, 121,5, color1);
            tft.frame(160, 120, 160, 121,5, highlight);
        }

        if(key==6){
            position++;
            if(position>1){
                position=1;
            }
            _delay_ms(100);
        }
        else if(key==4){
            position--;
            if(position<0){
                position=0;
            }
            _delay_ms(100);
        }

        else if(key==5)
        {
            if(position==1)
            {
                tft.setBackground(0xFFFF);
                tft.setForeground(0x0000);
                return 0;
            }
            else if(position==0)
            {
                tft.setBackground(0xFFFF);
                tft.setForeground(0x0000);
                return 1;
            }
        }
    }
}

void menu_background(void)  //This sets the background of the menu
{


    tft.setForeground(0xFFFF);//Font color

    tft.setZoom(2);	//text zoom

    tft.clean();

    tft.rectangle(0, 0, 160, 120, color1); //box is made
    tft.setBackground(color1); //backrgound color of the text
    tft.setXY(40,50); //position of the text
    puts("Message"); //text

    tft.rectangle(160, 0, 160, 120, color2);
    tft.setBackground(color2);
    tft.setXY(200,50);
    puts("Voice");


    tft.rectangle(0, 120, 160, 120, color3);
    tft.setBackground(color3);
    tft.setXY(20,170);
    puts("Test Call");

    tft.rectangle(160, 120, 160, 120, color4);
    tft.setBackground(color4);
    tft.setXY(180,170);
    puts("Test Alarm");



    return;
}

void send_message(void){
    return;
}

void text_menu(void){		//This is the menu over the text input screen
    tft.setZoom(1);

    tft.setXY(5,224);
    puts("Delete/Back");
    tft.setXY(240,224);
    puts("Accept/Send");
    tft.setXY(5,5);

    tft.setZoom(2);


    return;
}

void text_input(void){

    tft.setBackground(0xFFFF);
    tft.setForeground(0x0000);

    tft.clean();//cleans screen


    int i=32;  //This variable will be the ascii equivalent for the selected character
    char str[200];  //Declares string

    for(int k=0;k<200;k++){
        str[k]=0;  //Clears string
    }

    str[0]=i;  //First character of the string is 0
    str[1]='\0';  //Ends String
    int counter=0;  //Declares 0 as where to put the first character
    int key;	//Key pressed

    text_menu();
    puts(str);


    //Button 1: ABC1
    //Button 2: DEF2
    //Button 3: GHI3
    //Button 4: JKL4
    //Button 5: MNO5
    //Button 6: PQR6
    //Button 7: STU7
    //Button 8: VWX8
    //Button 9: YZ9
    //Button 0: Space 0
    //*: Cancel
    //#: Accept character, press twice to send

    while (1){
        rTouch::coord_t res;
        if (touch.pressed()){
        	res = touch.position();
        }
        if(res.x < 100 && res.y > 150){
           break;
        }


        key=getKey();

        if(key==1)			//If button  is pressed, next character is put at the last place in the string
        {
            tft.clean();
            i++;
            if(i==68){  //If the character count reaches the letter after the main 3, the number 1 is displayed
                i=49;
            }
            else if((i<65)||(i>67))  //Characters A->C
            {
                i=65;
            }

            str[counter]=i; //The letter is added to the main string
            str[counter+1]='\0';  //String is ended
            text_menu();  //Menu is overlayed
            puts(str);  //String is printed
            _delay_ms(100);  //Delay
        }
        else if(key==2)			//If button  is pressed, next character is put at the last place in the string
        {
            tft.clean();
            i++;
            if(i==71){
                i=50;
            }
            else if((i<68)||(i>70))
            {
                i=68;
            }

            str[counter]=i;
            str[counter+1]='\0';
            text_menu();
            puts(str);
            _delay_ms(100);  //Delay
        }
        else if(key==3)			//If button  is pressed, next character is put at the last place in the string
        {
            tft.clean();
            i++;
            if(i==74){
                i=51;
            }
            else if((i<71)||(i>73))
            {
                i=71;
            }

            str[counter]=i;
            str[counter+1]='\0';
            text_menu();
            puts(str);
            _delay_ms(100);  //Delay
        }
        else if(key==4)			//If button  is pressed, next character is put at the last place in the string
        {
            tft.clean();
            i++;
            if(i==77){
                i=52;
            }
            else if((i<74)||(i>76))
            {
                i=74;
            }

            str[counter]=i;
            str[counter+1]='\0';
            text_menu();
            puts(str);
            _delay_ms(100);  //Delay
        }
        else if(key==5)			//If button  is pressed, next character is put at the last place in the string
        {
            tft.clean();
            i++;
            if(i==80){
                i=53;
            }
            else if((i<77)||(i>79))
            {
                i=77;
            }

            str[counter]=i;
            str[counter+1]='\0';
            text_menu();
            puts(str);
            _delay_ms(100);  //Delay
        }
        else if(key==6)			//If button  is pressed, next character is put at the last place in the string
        {
            tft.clean();
            i++;
            if(i==83){
                i=54;
            }
            else if((i<80)||(i>82))
            {
                i=80;
            }

            str[counter]=i;
            str[counter+1]='\0';
            text_menu();
            puts(str);
            _delay_ms(100);  //Delay
        }
        else if(key==7)			//If button  is pressed, next character is put at the last place in the string
        {
            tft.clean();
            i++;
            if(i==86){
                i=55;
            }
            else if((i<83)||(i>85))
            {
                i=83;
            }

            str[counter]=i;
            str[counter+1]='\0';
            text_menu();
            puts(str);
            _delay_ms(100);  //Delay
        }
        else if(key==8)			//If button  is pressed, next character is put at the last place in the string
        {
            tft.clean();
            i++;
            if(i==89){
                i=56;
            }
            else if((i<86)||(i>88))
            {
                i=86;
            }

            str[counter]=i;
            str[counter+1]='\0';
            text_menu();
            puts(str);
            _delay_ms(100);  //Delay
        }
        else if(key==9)			//If button  is pressed, next character is put at the last place in the string
        {
            tft.clean();
            i++;
            if(i==91){
                i=57;
            }
            else if((i<89)||(i>90))
            {
                i=89;
            }

            str[counter]=i;
            str[counter+1]='\0';
            text_menu();
            puts(str);
            _delay_ms(100);  //Delay
        }
        else if(key==11)			//If button  is pressed, next character is put at the last place in the string
        {
            tft.clean();
            i++;
            if(i==33){
                i=48;
            }
            else if((i<32)||(i>32))
            {
                i=32;
            }

            str[counter]=i;
            str[counter+1]='\0';
            text_menu();
            puts(str);
            _delay_ms(100);  //Delay
        }




        if(key==12)
        {
            if(i==0)  //If button is pressed twice, i is already 0, so message is sent and user is sent back to the menu
            {
                int confirm =confirm_send();
                if(confirm)
                {
                    tft.clean();
                    menu_background();
                    break;
                }
            }
            else  //If button is pressed, current character is saved onto the string and next character is selected and reset to 0
            {
                counter++;
                i=0;
                _delay_ms(250);	//Delay
            }
        }

        if(key==10){
            if(counter==0)   //If there is nothing on the string, the user is sent back to the menu
            {
                tft.clean();
                menu_background();
                break;
            }
            if(i!=32||i!=0) //If the current character isn't empty, the character is deleted
            {
                i=32;
                str[counter]=i;
                text_menu();
                puts(str);
                _delay_ms(400);  //Delay

            }
            if(i==32||i==0)  //If the current character is empty, the character before is deleted
            {
                str[counter]='\0';
                counter--;
                str[counter]=32;
                tft.clean();
                text_menu();
                puts(str);
                _delay_ms(400);  //Delay
            }

        }

    }
	rTouch::coord_t res;
        if (touch.pressed())
           res = touch.position();
        
        if(res.x < 100 && res.y > 150){
	menu_background();        
        }
	/*if(res.x > 167 && res.y > 120 )
	{
		int confirm =confirm_send();
                if(confirm)
                {
                    tft.clean();
                    menu_background();
		}    	              		
	}*/
    
    return;
}

void panic(void){

    tft.setZoom(5);
    tft.clean();
    int key;


    while(1)
    {
        key=getKey();
        _delay_ms(200);  //Delay

        tft.rectangle(0, 0, 320, 240, 0xFFFF);  //Flashes between two screens
        tft.setBackground(0xFFFF);
        tft.setXY(60,50);
        tft.setForeground(0x0000);
        puts("HELP IS");
        tft.setXY(75,100);
        puts("NEEDED");


        tft.rectangle(0, 0, 320, 240, color4);
        tft.setBackground(color4);
        tft.setXY(60,50);
        tft.setForeground(0xFFFF);
        puts("HELP IS");
        tft.setXY(75,100);
        puts("NEEDED");

        if(key==5)
        {
            tft.clean();
            menu_background();
            break;
        }

    }


    return;
}

void voice(void){

    tft.setForeground(0xFFFF);
    tft.setZoom(3);
    tft.clean();

    tft.rectangle(0, 0, 320, 120, 0x0000);
    tft.setBackground(0x0000);
    tft.setXY(80,50);
    puts("Calling...");

    tft.rectangle(0, 120, 320, 120, color4);
    tft.frame(0, 120, 320, 121,5, highlight);
    tft.setBackground(color4);
    tft.setXY(100,170);
    puts("Hang Up");

    int key;

    while(1)
    {

        key=getKey();

        if(key==5)
        {
            tft.clean();
            menu_background();
            break;
        }
    }
    return;
}

void voice_incoming(void){

    tft.setForeground(0xFFFF);
    tft.setZoom(3);
    tft.clean();

    tft.rectangle(0, 0, 320, 120, 0x0000);
    tft.setBackground(0x0000);
    tft.setXY(45,50);
    puts("Call Incoming");

    tft.rectangle(0, 120, 160, 120, color2);
    tft.setBackground(color2);
    tft.setXY(30,170);
    puts("Accept");

    tft.rectangle(160, 120, 160, 120, color4);
    tft.setBackground(color4);
    tft.setXY(176,170);
    puts("Decline");

    int key;
    int position = 0;

    while(1)
    {

        key=getKey();


        if(position==0){
            tft.frame(0, 120, 160, 121,5, highlight);
            tft.frame(160, 120, 160, 121,5, color4);

        }
        else if(position==1){
            tft.frame(0, 120, 160, 121,5, color2);
            tft.frame(160, 120, 160, 121,5, highlight);
        }

        if(key==6){
            position++;
            if(position>1){
                position=1;
            }
            _delay_ms(100);
        }
        else if(key==4){
            position--;
            if(position<0){
                position=0;
            }
            _delay_ms(100);
        }

        else if(key==5)
        {
            if(position==1)
            {
                tft.clean();
                menu_background();
                break;
            }
            else if(position==0)
            {
                voice();
                break;
            }
        }
    }
    return;
}

void menu(void){

    int position=0;
    int key;
    menu_background();



    while(1){


        if (touch.pressed())
        {
            rTouch::coord_t res = touch.position();
            //printf("%d/%d, ",res.x,res.y);
            if (res.y > 165 && res.x < 120)
                voice();
            if (res.y < 165 && res.x < 120)
                text_input();
            if (res.y > 165 && res.x > 120)
                panic();
            if (res.y < 165 && res.x > 120)
                voice_incoming();
        }
        key = getKey();

        if(position==0){
            tft.frame(0, 0, 160, 121, 5, highlight);
            tft.frame(160, 0, 160, 121, 5, color2);
            tft.frame(0, 120, 160, 121, 5, color3);
            tft.frame(160, 120, 160, 121,5, color4);


            _delay_ms(100);  //Delay
        }
        else if(position==1){
            tft.frame(0, 0, 160, 121, 5, color1);
            tft.frame(160, 0, 160, 121, 5, highlight);
            tft.frame(0, 120, 160, 121, 5, color3);
            tft.frame(160, 120, 160, 121,5, color4);

            _delay_ms(100);  //Delay
        }
        else if(position==2){
            tft.frame(0, 0, 160, 121, 5, color1);
            tft.frame(160, 0, 160, 121, 5, color2);
            tft.frame(0, 120, 160, 121, 5, highlight);
            tft.frame(160, 120, 160, 121,5, color4);

            _delay_ms(100);  //Delay
        }
        else if(position==3){
            tft.frame(0, 0, 160, 121, 5, color1);
            tft.frame(160, 0, 160, 121, 5, color2);
            tft.frame(0, 120, 160, 121, 5, color3);
            tft.frame(160, 120, 160, 121,5, highlight);

            _delay_ms(100);  //Delay
        }


        if(position==0)		//Depending on the position, the movement buttons change accordingly
        {

            if(key==6){
                position=1;
            }
            else if(key==8){
                position=2;
            }
        }
        else if(position==1)
        {

            if(key==4){
                position=0;
            }

            else if(key==8){
                position=3;
            }
        }
        else if(position==2)
        {
            if(key==2){
                position=0;
            }

            else if(key==6){
                position=3;
            }

        }
        else if(position==3)
        {
            if(key==2){
                position=1;
            }
            else if(key==4){
                position=2;
            }

        }
        if(key==5)  //5 selects the current position
        {
            if (position == 0 )
            {
                text_menu();
            }
            else if (position==1)
            {
                voice();
            }
            else if (position==2)
            {
                voice_incoming();
            }
            else if (position==3)
            {
                panic();
            }
        }
    }
    return;
}

void security(void){
    int key;

    tft.setForeground(color1);
    tft.setZoom(3);
    tft.clean();

    tft.rectangle(0, 0, 320, 240, color1);
    tft.setBackground(color1);
    tft.setXY(33,70);
    tft.setForeground(0xFFFF);
    puts("Please confirm");
    tft.setXY(70,120);
    puts("Identity");

    while(1){

        key=getKey();


        if(key==5)
        {
            tft.clean();
            tft.rectangle(0, 0, 320, 240, color2);
            tft.setBackground(color2);
            tft.setXY(85,70);
            tft.setForeground(0xFFFF);
            puts("Identity");
            tft.setXY(80,120);
            puts("Confirmed");
            _delay_ms(1500);  //Delay
            tft.clean();
            menu();
        }
        else if(key==11)
        {
            tft.clean();
            tft.rectangle(0, 0, 320, 240, color4);
            tft.setBackground(color4);
            tft.setXY(35,70);
            tft.setForeground(0xFFFF);
            puts("USER NOT FOUND");
            _delay_ms(1500);  //Delay
            tft.clean();

            tft.rectangle(0, 0, 320, 240, color1);
            tft.setBackground(color1);
            tft.setXY(33,70);
            tft.setForeground(0xFFFF);
            puts("Please confirm");
            tft.setXY(70,120);
            puts("Identity");

        }

    }
    return;
}

int main(void){

    init();  //Initialize LCD
    tft.setOrient(tft.Landscape);  //Sets orientation


    tft.setBackground(color1);
    tft.setForeground(0xFFFF);
    tft.setZoom(1);
    /*
while(1){
if (touch.pressed())
        {
        rTouch::coord_t res = touch.position();
        printf("%d/%d, ",res.x,res.y);
_delay_ms(100);


}}
*/

    //security();  //Starts menu
    menu();

    return 0;
}



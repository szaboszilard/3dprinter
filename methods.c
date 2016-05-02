#define F_CPU 16000000UL// MOSI MISI MISO MOSÓMEDVE DDR
#define SPI_DDR DDRB
#include <avr/io.h>
#include  <util/delay.h>
#include "methods.h"
#include <avr/interrupt.h>
#include <inttypes.h>
#include <string.h>

typedef uint8_t u8;
typedef uint16_t u16;


extern char inbound;
extern char outbound;
extern int echo;

extern int x_freqvar;
extern int y_freqvar;
extern int z_freqvar;
extern int e_freqvar;

void init(){
DDRB|=0b10000000;//1 is output PB7 LED
DDRF|=0b11000101;//PF0 X_STEP; PF1 X_DIR; PF6 Y_STEP; PF7 Y_DIR; PF2 Y_EN_NOT
DDRC|=0b10000110;//PC7 E1_EN_NOT; PC3 E1_DIR; PC1 E1_STEP
DDRA|=0b01010100;//PA6 E0_DIR; PA4 E0_STEP; PA2 E0_EN_NOT
DDRD|=0b10000000;//PD7 X_EN_NOT
DDRL|=(1<<PL1|1<<PL3);
DDRK|=(1<<PK0);

}



void timer0_init()//simple timer setup
{

//ASSR = (1<<AS0);
TCCR0A = (1<<WGM01)
		|(0<<WGM00)
		|(0<<COM0A1)
		|(0<<COM0A0);

TCCR0B=	(0<<WGM02)
		|(0<<CS02)//16mHz ext clk
		|(1<<CS01)
		|(0<<CS00);


OCR0A=20;
			//should produce around 100 hz
TIMSK0 = (1<<OCIE0A);//|(1<<TOIE0);
}



void timer1_init()//for Z axis
{

//ASSR = (1<<AS0);

TCCR1A = (0<<WGM11)
		|(0<<WGM10)
		|(0<<COM1A1)
		|(0<<COM1A0);

TCCR1B=	(0<<WGM13)
		|(1<<WGM12)
		|(0<<CS12)//16mHz ext clk /8
		|(1<<CS11)
		|(0<<CS10);

TCCR1C|=0x00;

OCR1A=z_freqvar;
//OCR1AH=0x4E; //4E				//should produce around 100 hz
//OCR1AL=0x20; //20				//should produce around 100 hz
TIMSK1 = (1<<OCIE1A);//|(1<<TOIE0);
}



void timer3_init()//for x axis
{

//ASSR = (1<<AS0);

TCCR3A = (0<<WGM31)
		|(0<<WGM30)
		|(0<<COM3A1)
		|(0<<COM3A0);

TCCR3B=	(0<<WGM33)
		|(1<<WGM32)
		|(0<<CS32)//16mHz ext clk /8
		|(1<<CS31)
		|(0<<CS30);

TCCR3C|=0x00;

OCR3A=x_freqvar;
//OCR1AH=0x4E; //4E				//should produce around 100 hz
//OCR1AL=0x20; //20				//should produce around 100 hz
TIMSK3 = (1<<OCIE3A);//|(1<<TOIE0);
}



void timer4_init()//for y axis
{

//ASSR = (1<<AS0);

TCCR4A = (0<<WGM41)
		|(0<<WGM40)
		|(0<<COM4A1)
		|(0<<COM4A0);

TCCR4B=	(0<<WGM43)
		|(1<<WGM42)
		|(0<<CS42)//16mHz ext clk /8
		|(1<<CS41)
		|(0<<CS40);

TCCR4C|=0x00;

OCR4A=y_freqvar;
//OCR1AH=0x4E; //4E				//should produce around 100 hz
//OCR1AL=0x20; //20				//should produce around 100 hz
TIMSK4 = (1<<OCIE4A);//|(1<<TOIE0);
}



void timer5_init()//for extruder
{

//ASSR = (1<<AS0);

TCCR5A = (0<<WGM51)
		|(0<<WGM50)
		|(0<<COM5A1)
		|(0<<COM5A0);

TCCR5B=	(0<<WGM53)
		|(1<<WGM52)
		|(0<<CS52)//16mHz ext clk /8
		|(1<<CS51)
		|(0<<CS50);

TCCR5C|=0x00;

OCR5A=e_freqvar;
//OCR1AH=0x4E; //4E				//should produce around 100 hz
//OCR1AL=0x20; //20				//should produce around 100 hz
TIMSK5 = (1<<OCIE5A);//|(1<<TOIE0);
}



void lcd_init(){
DATA_DDR |= 0XF0;//determine data direction
COMM_DDR |= (1<<CMD_RS)|(1<<CMD_RW)|(1<<CMD_EN);
}
void adc_init(){
	DDRF&=~(1<<PF0);
	ADMUX|=(1<<REFS0);

	ADCSRA|=(1<<ADEN)
			|(1<<ADPS1)
			|(1<<ADPS2);
}



/*void SPI_init(int select)
{ÿ	if(select == MASTER)
{
	SPCR |= (1<<SPE)|(1<<MSTR)|(1<<SPR1);

		SPI_DDR |= (1<<SPI_SS);
		SPI_DDR |= (1<<SPI_SCK);
		SPI_DDR |= (1<<SPI_MOSI);
		SPI_DDR |= (1<<SPI_CS);

		SPI_DDR &=~(1<<SPI_MISO);
	}
}
*/

void SPI_write(int input)
{
CS_EN; // select slave
SPDR = input;
while(!(SPSR&(SPIF)));
}

unsigned char SP1_read()
{
CS_DTS;
while(!(SPSR&(SPIF)));
return SPDR;
}

int ADC_read(int ch)
{
ADMUX&=0xE0;
ADMUX|=ch&0x1F;

ADCSRA |= (1<<ADSC);

while(!(ADCSRA&(1<<ADIF)));

return ADCW;
}

void led_out(int led){
 PORTB=(led<<4);
 PORTD=led;
}

int led_read(char ports[])
{
int led=0;
	if ((strcmp(ports,"B,D"))==0)
	{
	led=PORTD;
	led|=PORTB>>4;
	}
return led;
}

int G_port_button(){
return PING;
}


void Segment_Place(int place,int number){
PORTA=0X80 | (place<<4) | number;
}


void My_Segment_Write(int number,int del)
{
	if(number==0)
		{
		Segment_Place(0,0);
		_delay_ms(del);
		Segment_Place(1,0);
		_delay_ms(del);
		Segment_Place(2,0);
		_delay_ms(del);
		Segment_Place(3,0);
		}
	else
		{
		Segment_Place(0,number%10);
		_delay_ms(del);

		Segment_Place(1,number/10%10);
		_delay_ms(del);

		Segment_Place(2,number/100%10);
		_delay_ms(del);

		Segment_Place(3,number/1000%10);
		_delay_ms(del);
		}

}


void Segment_Write(int number)
{
	switch(digit)
	{
	case 0:

	Segment_Place(0,number%10);
	break;


	case 1:

	Segment_Place(1,number/10%10);
	break;


	case 2:

	Segment_Place(2,number/100%10);
	break;


	case 3:

	Segment_Place(3,number/1000%10);
	break;

	}
	digit++;
	if(digit==4)
	{
	digit=0;
	}

}

int read_out()	//returns 0-12 button else 13
{
int row=0;
int column=0;
for(row=0;row<4;row++)
	{
	PORTC=(1<<(row+3)); // shifting to become output
	_delay_ms(1); 
	column=PINC & 0x07; // 
	switch(column)
		{
			case 6:
			return((row*3)+1);
			break;

			case 5:
			if(row==3) {return 0;}
			else {return ((row*3)+2);}
			break;

			case 3:
			return((row*3)+3);
			break;
		}
	}
	return 13;
}

void rgb_led_out(int input) //1: red 2: green 4: blue
{
	switch (input)
	{
	case 1:
	PORTC=0x80;
	//_delay_ms(200);
	break;

	case 2:
	PORTE=0x04;
	break;

	case 4:
	PORTE=0x08;
	break;

	default:
	PORTC=0;
	PORTE=0;
	break;
	}
}

void lcdsenddata(int c)//upper four bits are used to send data
{
COMM_PORT |= (1<<CMD_RS);//instruction register set to send data
DATA_PORT = c & 0xF0;
clock();
DATA_PORT = c<<4;
clock();
}


void lcdsendcommand(int c)//upper four bits are used to send data
{
COMM_PORT &= ~(1<<CMD_RS);//toggles the bit
DATA_PORT = c & 0xF0;
clock();
DATA_PORT = c<<4;
clock();
}


void clock()
{
	_delay_ms(3);
	COMM_PORT |= (1<<CMD_EN);
	_delay_ms(3);
	COMM_PORT &= ~(1<<CMD_EN);
}

void lcdconfig()
{
lcd_init();
COMM_PORT &= ~(1<<CMD_RW); // RW is 0 then writes on the screen
COMM_PORT &= ~(1<<CMD_RS); //RS is 0 then it sends commands

DATA_PORT |= 0X20; // uses the LCD  in 4 bit mode

lcdsendcommand(0x28);//4 bit mode with 2 lines
lcdsendcommand(0x28);//4 bit mode with 2 lines
lcdsendcommand(0x28);//4 bit mode with 2 lines

lcdsendcommand(0x02);// default / reset mode
lcdsendcommand(0x01);// clear
lcdsendcommand(0x0F);// cursor blink wtf
}

void my_lcd_string(char *pointer,int line, int pos) //4x16
{
	int lineposvar=0;

	lcdsendcommand(0x01);
	
	if(line<5)
	{
	switch (line)
		{
		case 1:
		lineposvar=0x00;
		break;

		case 2:
		lineposvar=0x40;
		break;

		case 3:
		lineposvar=0x10;
		break;

		case 4:
		lineposvar=0x50;
		break;

		}
	}
	
	
	if(pos<(0x10))
	{
	lineposvar+=pos;
	}
	
	lineposvar|=0x80;

	lcdsendcommand(lineposvar);

//	lcdsendcommand(0x0F);
		
	while(*pointer)
		{
			lcdsenddata(*pointer);
			pointer++;
		}
}

void my_lcd_string2(char *pointer,int line, int pos) //4x16
{
	int lineposvar=0;

	lcdsendcommand(0x01);
	
	if(line<5)
	{
	switch (line)
		{
		case 1:
		lineposvar=0x00;
		break;

		case 2:
		lineposvar=0x40;
		break;

		case 3:
		lineposvar=0x10;
		break;

		case 4:
		lineposvar=0x50;
		break;

		}
	}
	
	
	if(pos<(0x10))
	{
	lineposvar+=pos;
	}
	
	lineposvar|=0x80;

	lcdsendcommand(lineposvar);

//	lcdsendcommand(0x0F);
		
	while(*pointer)
		{
			lcdsenddata(*pointer);
			pointer++;
		}
}

void lcd_string(char *pointer) //4x16
{
		
	while(*pointer)
		{
			lcdsenddata(*pointer);
			pointer++;
		}
}
void my_lcd_numbers(u16 number,int line, int pos) //4x16
{
	u16 bup;
	int lineposvar=0;
	int numnum=0;
	
	bup=number;

	lcdsendcommand(0x01);
	
	if(line<5)
	{
	switch (line)
		{
		case 1:
		lineposvar=0x00;
		break;

		case 2:
		lineposvar=0x40;
		break;

		case 3:
		lineposvar=0x10;
		break;

		case 4:
		lineposvar=0x50;
		break;

		}
	}
	
	
	if(pos<(0x10))
	{
	lineposvar+=pos;
	}
	
	lineposvar|=0x80;

	lcdsendcommand(lineposvar);

//	lcdsendcommand(0x0F);

	while(number)
		{
			number=number/10;
			numnum++;
		}
		
	while(number)
		{
			lcdsenddata(number%10+48);
			number=number/10;
		}
}


void lcd_move(int row, int column)
{
//0x00
//0x40
//0x10
//0x50

int tmp = 0;

int line0=0x00;
int line1=0x40;
int line2=0x10;
int line3=0x50;

switch(row)
{
	case 0:
	tmp=(line0 + 0x80 + column);
	break;

	case 1:
	tmp=(line1 + 0x80 + column);
	break;

	case 2:
	tmp=(line2 + 0x80 + column);
	break;

	case 3:
	tmp=(line3 + 0x80 + column);
	break;

	default:
	tmp=(line0 + 0x80 + column);
	break;


}
lcdsendcommand(tmp);
}

void lcd_num(unsigned long number)
{
lcdsenddata('0' + (number/10000000)%10);
lcdsenddata('0' + (number/1000000)%10);
lcdsenddata('0' + (number/100000)%10);
lcdsenddata('0' + (number/10000)%10);
lcdsenddata('0' + (number/1000)%10);
lcdsenddata('0' + (number/100)%10);
lcdsenddata('0' + (number%10));
}

void buzzer_on() //also lights up blue light
{
DDRE|=(1<<PE3);
TCCR3A=((1<<COM3A1)|(1<<WGM30));
TCCR3B|=((1<<WGM32)|(1<<CS32));
//PRESCALE IS 256;
OCR3A=128;
}


void buzzer_off()
{
TCCR3A&=~(1<<COM3A1);
}


// line adresses
//0. 0x00
//1. 0x40
//2. 0x10
//3. 0x50



///custom chars go into CGRAM L0 _ _ _ _ _
//addr:0 1 2 3 4 5
//0x40 0x48 0x50 0x55 0x60 0x68
void Createmyobject00()
{
lcdsendcommand(0x40);//sets CG RAM address 0
lcdsenddata(0b01111);
lcdsenddata(0b01111);
lcdsenddata(0b00110);
lcdsenddata(0b11111);
lcdsenddata(0b11111);
lcdsenddata(0b01110);
lcdsenddata(0b01110);
lcdsenddata(0b01110);
}

void selectmyobject(int digit)
{
lcdsenddata(digit); // wit this we select our cgram object;
}


//uart: PE0 pin rx,PE1 tx
//UCSRnA WE NEVER USE
//UCSRnB we use
//RX complete interrupt
//TX complete interrupt
//USART data register empty inter
//USART RXENn
//USART TXENn
// we don't set char sizes
//1 setting can double the baud rate
//we just use U2X=0

void UART_init()
{
	UCSR0A=0;//CAUSE WE NEVER USE IT
	UCSR0B|=(1<<RXEN0)  //enable receiving
		 |(1<<TXEN0);  //enable transmission

	UCSR0C|=(1<<UCSZ01) //transmit and send 8 bits
		  |(1<<UCSZ00);

	UBRR0L=103; //96k baud rate with 16 mhz (51 with 8 mhz)
	UBRR0H=0;

	DDRE|=(1<<PE1)
		|(1<<PE1);
}


void USART_Transmit( unsigned char data )
{
/* Wait for empty transmit buffer */
while ( !( UCSR0A & (1<<UDRE0)) )
;
/* Put data into buffer, sends the data */
UDR0 = data;
}

unsigned char USART_Receive( void )
{
/* Wait for data to be received */
while ( !(UCSR0A & (1<<RXC0)) )
;
/* Get and return received data from buffer */
return UDR0;									//+48 and you will get the actual character
}


void USART_String_Transmit( char* pointer )
{
/* Wait for empty transmit buffer */
while(*pointer)
{
USART_Transmit(*pointer);
pointer++;
}
}

void lcd_animated_char1(int start, int delay)
{
int i;
	while(start)
	{
		lcdsendcommand(0x40);//clears lcd
		_delay_ms(5);
		lcd_move(0,0);
		_delay_ms(5);

		lcdsendcommand(0x40);//sets CG RAM address 0
		lcdsenddata(0b11111);
		lcdsenddata(0b10001);
		lcdsenddata(0b10001);
		lcdsenddata(0b10001);
		lcdsenddata(0b10001);
		lcdsenddata(0b10001);
		lcdsenddata(0b10001);
		lcdsenddata(0b11111);

for(i=0;i<100;i++)
{
		selectmyobject(0);
		lcdsendcommand(0x01);
}
		_delay_ms(delay);



		lcdsendcommand(0x40);//clears lcd
		_delay_ms(5);
		lcd_move(0,0);
		_delay_ms(5);

		lcdsendcommand(0x40);//sets CG RAM address 0
		lcdsenddata(0b11111);
		lcdsenddata(0b11111);
		lcdsenddata(0b11011);
		lcdsenddata(0b11011);
		lcdsenddata(0b11011);
		lcdsenddata(0b11011);
		lcdsenddata(0b11111);
		lcdsenddata(0b11111);

for(i=0;i<100;i++)
{
		selectmyobject(0);
		lcdsendcommand(0x01);
}


		_delay_ms(delay);



		lcdsendcommand(0x40);//clears lcd
		_delay_ms(5);
		lcd_move(0,0);
		_delay_ms(5);

		lcdsendcommand(0x40);//sets CG RAM address 0
		lcdsenddata(0b11111);
		lcdsenddata(0b11111);
		lcdsenddata(0b11111);
		lcdsenddata(0b11111);
		lcdsenddata(0b11111);
		lcdsenddata(0b11111);
		lcdsenddata(0b11111);
		lcdsenddata(0b11111);

		
for(i=0;i<100;i++)
{
		selectmyobject(0);
		lcdsendcommand(0x01);
}

		_delay_ms(delay);
	}

}


void matrix_to_led(int matrix)
{
led_out(matrix);
}

void UART_cmd()
{
int i=0;
char c;
char ac[50];

if(i<5)
{
	if(c^=inbound)
	{
	ac[i]=inbound+48;
	i++;
	}
		else
		{
		ac[i+1]='\0';

			while(1)
			{
			my_lcd_string(ac,0,0);
			}
		}


}
}


int fetchint(char *t,int p, int ch)//ex:command[],i! or !comm,'X' ONLY unssigned
{
int myint=0;

int a=p;//front to letter
int b=p;	 //moving var
int c=p;	//last digit
int i=0; //incrementing exponent

			if(echo)
			{
			USART_String_Transmit("\n*Fetching int*");
			}

	while(!(t[a]==ch))
	{
	a++;
	USART_Transmit(a+48);
		if(a>(58-p))
		{
		return -1;
		}
	}
	
	c=a+1; //a number	

	while((t[c]>47)&&(t[c]<58))//goes exactly till
	{
	c++;
	}

	c--;//but cwas a+1

	for(b=c;b>a;b--)//goes till a+1
	{
		if(i==0)
		{
		myint+=(t[b]-48);
		
			if(echo)
			{
			USART_String_Transmit("\n*Int: 1*");
			}
		}

		if(i==1)
		{
		myint+=(t[b]-48)*10;
		
			if(echo)
			{
			USART_String_Transmit("\n*Int: 10*");
			}
		}

		if(i==2)
		{
		myint+=(t[b]-48)*100;
			
			if(echo)
			{
			USART_String_Transmit("\n*Int: 100*");
			}
		}

		if(i==3)
		{
		myint+=(t[b]-48)*1000;
		
			if(echo)
			{
			USART_String_Transmit("\n*Int: 1K*");
			}
		}

		if(i==4)
		{
		myint+=(t[b]-48)*10000;
			if(echo)
			{
			USART_String_Transmit("\n*Int: 10K*");
			}
		}


	i++;
	}						

return myint;

}




long fetchlong(char *t,int p, int ch)//ex:command[],i! or !comm,'X' ONLY unsigned
{
long mylong=0;

int a=p;//front to letter
int b=p;	 //moving var
int c=p;	//last digit
int i=0; //incrementing exponent

			if(echo)
			{
			USART_String_Transmit("\n*Fetching long*");
			}

	while(!(t[a]==ch))
	{
	a++;
	USART_Transmit(a+48);
		if(a>(58-p))
		{
		return -1;
		}
	}
	
	c=a+1; //a number	

	while((t[c]>47)&&(t[c]<58))//goes exactly till
	{
	c++;
	}

	c--;//but cwas a+1

	for(b=c;b>a;b--)//goes till a+1
	{
		if(i==0)
		{
		mylong+=(t[b]-48);
		
			if(echo)
			{
			USART_String_Transmit("\n*Long: 1*");
			}
		}

		if(i==1)
		{
		mylong+=(t[b]-48)*10;
		
			if(echo)
			{
			USART_String_Transmit("\n*Long: 10*");
			}
		}

		if(i==2)
		{
		mylong+=(t[b]-48)*100;
			
			if(echo)
			{
			USART_String_Transmit("\n*Long: 100*");
			}
		}

		if(i==3)
		{
		mylong+=(t[b]-48)*1000;
		
			if(echo)
			{
			USART_String_Transmit("\n*Long: 1K*");
			}
		}

		if(i==4)
		{
		mylong+=(t[b]-48)*10000;
			if(echo)
			{
			USART_String_Transmit("\n*Long: 10K*");
			}
		}

		if(i==5)
		{
		mylong+=(t[b]-48)*100000;
		
			if(echo)
			{
			USART_String_Transmit("\nLong: 100K*");
			}
		}

		if(i==6)
		{
		mylong+=(t[b]-48)*1000000;
			
			if(echo)
			{
			USART_String_Transmit("\n*Long: 1M*");
			}
		}

		if(i==7)
		{
		mylong+=(t[b]-48)*10000000;
			
			if(echo)
			{
			USART_String_Transmit("\n*Long 10 M*");
			}
		}

		if(i==8)
		{
		mylong+=(t[b]-48)*100000000;
			
			if(echo)
			{
			USART_String_Transmit("\n*Long 100 M*");
			}
		}

		if(i==9)
		{
		mylong+=(t[b]-48)*1000000000; //grants about 7 years of printing if 1 line=1 sec 2 147 483 647 is the limit
			
			if(echo)
			{
			USART_String_Transmit("\n*Long 1Bill*");
			}
		}

	i++;
	}						

return mylong;

}




double threefetchdouble(char *t,int p, int ch)//ex:command[],i! or !comm,'X' ONLY unsigned format xxxx.xxx if there is no 'X' returns -1
{
double mydouble=0;

int a=p;//front to letter
int b=p;	 //moving var
int c=p;	//last digit
int i=0; //incrementing exponent

			if(echo)
			{
			USART_String_Transmit("\n*3Fetching double*");
			}

	while(!(t[a]==ch))
	{
	a++;
		if(echo)
		{
		USART_Transmit(a+48);
		}

		if(a>(58-p))
		{
		return -1.0;
		}
	}
	
	c=a+1; //a number	

	while(((t[c]>47)&&(t[c]<58))||(t[c]=='.'))//goes exactly till
	{
	c++;
	}

	c--;//but cwas a+1

	for(b=c;b>a;b--)//goes till a+1
	{
		if(i==0)
		{
		mydouble+=(double)(t[b]-48)*0.001;
		
			if(echo)
			{
			USART_String_Transmit("\n*3Double 0.001*");
			}
		}

		if(i==1)
		{
		mydouble+=(double)(t[b]-48)*0.010;
		
			if(echo)
			{
			USART_String_Transmit("\n*3Double 0.010*");
			}
		}

		if(i==2)
		{
		mydouble+=(double)(t[b]-48)*0.100;
			
			if(echo)
			{
			USART_String_Transmit("\n*3Double 0.100*");
			}
		}

		if(i==3)
		{
		
			if(echo)
			{
			USART_String_Transmit("\n*3Double [dot]*");
			}
		}

		if(i==4)
		{
		mydouble+=(double)(t[b]-48)*1.000;
			if(echo)
			{
			USART_String_Transmit("\n*3Double 1*");
			}
		}

		if(i==5)
		{
		mydouble+=(double)(t[b]-48)*10.000;
		
			if(echo)
			{
			USART_String_Transmit("\n3Double 10*");
			}
		}

		if(i==6)
		{
		mydouble+=(double)(t[b]-48)*100.000;
			
			if(echo)
			{
			USART_String_Transmit("\n*3Double 100*");
			}
		}

		if(i==7)
		{
		mydouble+=(double)(t[b]-48)*1000.000;
			
			if(echo)
			{
			USART_String_Transmit("\n*3Float 1K*");
			}
		}

		if(i==8)
		{
		mydouble+=(double)(t[b]-48)*10000.000;
			
			if(echo)
			{
			USART_String_Transmit("\n*3Float 10K*");
			}
		}

		if(i==9)
		{
		mydouble+=(double)(t[b]-48)*100000.000; //grants about 7 years of printing if 1 line=1 sec 2 147 483 647 is the limit
			
			if(echo)
			{
			USART_String_Transmit("\n*3Double 100K*");
			}
		}

	i++;
	}						

return mydouble;

}


double fourfetchdouble(char *t,int p, int ch)//ex:command[],i! or !comm,'X' ONLY unsigned format xxxx.xxxx returns -1 if no char
{
double mydouble=0;

int a=p;//front to letter
int b=p;	 //moving var
int c=p;	//last digit
int i=0; //incrementing exponent

			if(echo)
			{
			USART_String_Transmit("\n*4Fetching double*");
			}

	while(!(t[a]==ch))
	{
	a++;
		if(echo)
		{
		USART_Transmit(a+48);
		}

		if(a>(58-p))
		{
		return -1.0;
		}
	}
	
	c=a+1; //a number	

	while(((t[c]>47)&&(t[c]<58))||(t[c]=='.'))//goes exactly till
	{
	c++;
	}

	c--;//but cwas a+1

	for(b=c;b>a;b--)//goes till a+1
	{
		if(i==0)
		{
		mydouble+=(double)(t[b]-48)*0.0001;
		
			if(echo)
			{
			USART_String_Transmit("\n*4Double 0.0001*");
			}
		}

		if(i==1)
		{
		mydouble+=(double)(t[b]-48)*0.0010;
		
			if(echo)
			{
			USART_String_Transmit("\n*4Double 0.0010*");
			}
		}

		if(i==2)
		{
		mydouble+=(double)(t[b]-48)*0.0100;
			
			if(echo)
			{
			USART_String_Transmit("\n*4Double 0.0100*");
			}
		}

		if(i==3)
		{
		mydouble+=(double)(t[b]-48)*0.1000;
		
			if(echo)
			{
			USART_String_Transmit("\n*4Double 0.1000*");
			}
		}

		if(i==4)
		{
		
			if(echo)
			{
			USART_String_Transmit("\n*3Double [dot]*");
			}
		}

		if(i==5)
		{
		mydouble+=(double)(t[b]-48)*1.0000;
		
			if(echo)
			{
			USART_String_Transmit("\n4Double 1*");
			}
		}

		if(i==6)
		{
		mydouble+=(double)(t[b]-48)*10.0000;
			
			if(echo)
			{
			USART_String_Transmit("\n*4Double 10*");
			}
		}

		if(i==7)
		{
		mydouble+=(double)(t[b]-48)*100.0000;
			
			if(echo)
			{
			USART_String_Transmit("\n*4Float 100 *");
			}
		}

		if(i==8)
		{
		mydouble+=(double)(t[b]-48)*1000.000;
			
			if(echo)
			{
			USART_String_Transmit("\n*4Double 1K*");
			}
		}

		if(i==9)
		{
		mydouble+=(double)(t[b]-48)*10000.000; //grants about 7 years of printing if 1 line=1 sec 2 147 483 647 is the limit
			
			if(echo)
			{
			USART_String_Transmit("\n*4Double 10K*");
			}
		}

	i++;
	}						

return mydouble;

}

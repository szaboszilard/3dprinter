#define F_CPU 16000000UL
#define SPI_DDR DDRB

#include <avr/io.h> 
#include  <util/delay.h>
#include <math.h>
#include <avr/interrupt.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include "methods.h"


typedef uint8_t u8;
typedef uint16_t u16;

unsigned int val1=0;
unsigned int val2=60;
int ttoglenum=0;
int incrnum=0;

volatile int echo=1;
volatile char outbound;
volatile char inbound;

volatile int tick01=0;
volatile int tick02=0;
volatile int tick0s=0;
volatile int tick0m=0;
volatile int tick0h=0;
volatile int tick0d=0;

//GLOBAL MANIPULATOR VARIABLES*******************************
volatile int maxtemp_ext=270;
volatile int maxtemp_bed=150;
volatile int ext_temp;
volatile int bed_temp;
volatile int tgttemp_ext;
volatile int tgtemp_bed;

volatile long x_max=12929;
volatile long y_max=15279;
volatile long z_max=240000;
volatile long e_max=-1;

volatile long current_x=0;
volatile long current_y=0;
volatile long current_z=0;
volatile long current_e=0;

volatile double double_current_x=0;
volatile double double_current_y=0;
volatile double double_current_z=0;
volatile double double_current_e=0;


volatile double double_scan_x=-1.0;
volatile double double_scan_y=-1.0;
volatile double double_scan_z=-1.0;
volatile double double_scan_e=-1.0;

volatile double double_temp_x=0.0;
volatile double double_temp_y=0.0;
volatile double double_temp_z=0.0;
volatile double double_temp_e=0.0;

volatile double double_xy_length=0;
volatile double slope=0;
volatile double angle=0;
volatile double x_square=0;
volatile double y_square=0;
volatile double root=0;

volatile long temp_x=0;
volatile long temp_y=0;
volatile long temp_z=0;
volatile long temp_e=0;

volatile long target_x=0;
volatile long target_y=0;
volatile long target_z=0;
volatile long target_e=0;

volatile double plan_x=0.0;
volatile double plan_y=0.0;
volatile double plan_z=0.0;
volatile double plan_e=0.0;

volatile int x_target_freq=0;
volatile int y_target_freq=0;
volatile int z_target_freq=0;
volatile int e_target_freq=0;
volatile int xy_target_freq=0;

volatile double x_double_freq=0;
volatile double y_double_freq=0;
volatile double z_double_freq=0;
volatile double e_double_freq=0;
volatile double xy_double_freq=0;

volatile int xroll=0;
volatile int xstop=0;
volatile int xdir=0;

volatile int yroll=0;
volatile int ystop=0;
volatile int ydir=0;

volatile int zroll=0;
volatile int zstop=0;
volatile int zdir=0;

volatile int eroll=0;
volatile int estop=0;
volatile int edir=0;
volatile double xy_double_length=0;
volatile long xy_legth=0;
volatile double e_time=0;

volatile char command[58];

volatile long line_number=0;
volatile long temp_line_number=0;

volatile int comm=0;

volatile int ca;
volatile int cb;
volatile int cc;

volatile int mode=0;

volatile int testval=0;

//-----------for 2 cm/s except z axis
volatile int z_freq=3200;
volatile int z_freqvar=6250;//16M/8/30k would be 62.5 so max error 3k was a bit too much so it's 2mm/s now
volatile int e_max_freq=2000;
volatile int e_min_freqvar=667;//about 1.5 error
volatile int e_freqvar=667;

volatile int xy_max_freq=1600;
volatile int xy_freqvar=1250;//exact
volatile int x_freqvar=1250;
volatile int y_freqvar=1250;

//-----------for 2 cm/s
//***********************************************************


volatile char dub1[50];
volatile char dub2[50];

int rnr=0;

int td0 = 1;


ISR(TIMER0_COMPA_vect)
{
tick01++;
	if(tick01==1000)//100khz
		{
		tick01=0;
		tick02++;//100HZ
		}
		if(tick02==100)
		{
		tick02=0;
		tick0s++;//1sec
			if(echo)
			{
			USART_Transmit(' ');
			}
		}
		if(tick0s==60)
		{
		tick0s=0;
		tick0m++;//1 min
		}
		if(tick0m==60)
		{
		tick0m=0;
		tick0h++;//1 hour
		}
		if(tick0h==24)
		{
		tick0h=0;
		tick0d++;//1day
		}
}

ISR(TIMER1_COMPA_vect) //for Z axis
{
cli();

	if(zdir==0)//REVERSE direction
	{
		PINL|=(1<<PL1);
			if(current_z==target_z)
			{
			zroll=0;
			}

		if(zroll&&(target_z<current_z))
		{
			PINL|=(1<<PL3);

			current_z--;


			if(current_z==target_z)
			{
			zroll=0;
			}
		}
	}	

	if(zdir==1)//normal diretion
	{

		if(zroll&&(target_z>current_z))
		{
		PINL|=(1<<PL3);

		current_z++;


			if(current_z==target_z)//moves one then stops
			{
			zroll=0;
			}
		}
	}

sei();
}



ISR(TIMER3_COMPA_vect) //for X axis
{
cli();

	if(xdir==1)//REVERSE direction
	{
	PINF|=0b00000010;

			if(current_x==target_x)
			{
			xroll=0;
			}

		if(xroll&&(target_x<current_x)&&(zroll==0))
		{
			PINF|=0b00000001;

			current_x--;


			if(current_x==target_x)
			{
			xroll=0;
			if(echo)
			{
			USART_String_Transmit("\n\rcurrent_X decreased to targetX");
			}
			}
		}
	}	

	if(xdir==0)//normal diretion
	{
		if(xroll&&(target_x>current_x)&&(zroll==0))
		{
		PINF|=0b00000001;

		current_x++;


			if(current_x==target_x)//moves one then stops
			{
			xroll=0;
			if(echo)
			{
			USART_String_Transmit("\n\rcurrent_X increased to targetX");
			}
			}
		}
	}

sei();
}


ISR(TIMER4_COMPA_vect) //for Y axis
{
cli();

	if(current_y==target_y)
	{
	yroll=0;
	}	

	if(ydir==1)//REVERSE DIRECTION
	{
	PINF|=0b10000000;

		if(yroll&&(target_y<current_y)&&(zroll==0))
		{
		PINF|=0b01000000;
		current_y--;

				if(current_y==target_y)
				{
				yroll=0;
				}
		}
	}

	if(ydir==0)//NORMAL DIRECTION
	{

		if(yroll&&(target_y>current_y)&&(zroll==0))
		{
		PINF|=0b01000000;
		current_y++;

				if(current_y==target_y)
				{
				yroll=0;
				}
		}
	}
sei();
}




ISR(TIMER5_COMPA_vect) //for EXTRUDER
{
cli();

	if(edir==1)
	{
	PINA|=0b01000000;
	

	if(eroll&&(target_e>current_e)&&(zroll==0))
	{

		PINA|=0b00010000;

	current_e++;

		if(current_e==target_e)
		{
		eroll=0;
		}
	}
	}
sei();

}




void line_engine(double double_scan_x,double double_scan_y, double double_scan_z, double double_scan_e, int i)
{	
	cli();
	if(echo)
	{
	USART_String_Transmit("\nIn line engine.");
	}
	
	double_scan_x=threefetchdouble(command,i,'X');
	double_scan_y=threefetchdouble(command,i,'Y');
	double_scan_z=threefetchdouble(command,i,'Z');//scans inputs
	double_scan_e=fourfetchdouble(command,i,'E');

	
	
//----------------------------------setting z, height

	if(double_scan_z>0)
	{
	target_z=(double_scan_z*1600.000);
		if(target_z>current_z)
		{
		
		zdir=1;
		zroll=1;
			if(echo)
			{
			USART_String_Transmit("\nzroll=1");
			USART_String_Transmit("\nzdir=1");
			}
		}

		if(target_z<current_z)
		{
		zdir=0;
		zroll=1;
			if(echo)
			{
			USART_String_Transmit("\nzroll=1");
			USART_String_Transmit("\nzdir=0");
			USART_String_Transmit("\nIt's smaller");
			}

		}
		
	}
	else
	{
	target_z=current_z;
	}
	


//----------------------------------setting x and y

	double_current_x=(double)((double)current_x/80.000);
	double_current_y=(double)((double)current_y/80.000);
/*
if(echo)
{
snprintf(dub1,50,"%f",double_current_x);
USART_String_Transmit("\n\n\rdub1:");
USART_String_Transmit(dub1);
snprintf(dub2, 50, "%f", double_current_y);
USART_String_Transmit("\n\n\rdub1:");
USART_String_Transmit(dub2);
USART_String_Transmit("\n\n\r");
}
*/
//----------------------
	if(double_scan_x>0&&double_scan_y>0)//neither of them are 0
	{
		if(double_scan_x>double_current_x&&double_scan_y>double_current_y)//from up left to down right 1.
		{
		xdir=0;//xdir+
		ydir=0;//ydir+

		double_temp_x=(double)(double_scan_x-double_current_x);//scan-current cause scan is larger
		double_temp_y=(double)(double_scan_y-double_current_y);//scan-current cause scan us larger

		}

		if(double_scan_x<double_current_x&&double_scan_y>double_current_y)//down left to up right 2.
		{
		xdir=1;//xdir-
		ydir=0;//ydir+

		double_temp_x=(double)(double_current_x-double_scan_x);//current_scan cause current is larger!!!!
		double_temp_y=(double)(double_scan_y-double_current_y);//scan-current cause scan us larger
		}
		
		if(double_scan_x>double_current_x&&double_scan_y<double_current_y)//up right to down left 3.
		{
		xdir=0;//xdir+
		ydir=1;//ydir-

		double_temp_x=(double)(double_scan_x-double_current_x);//scan-current cause scan is larger
		double_temp_y=(double)(double_current_y-double_scan_y);//current-scan cause current us larger!!!
		}
		
		if(double_scan_x<double_current_x&&double_scan_y<double_current_y)//down right to up left 4.
		{
		xdir=1;//xdir-
		ydir=1;//ydir-

		double_temp_x=(double)(double_current_x-double_scan_x);//current-scan cause scan is larger!!!
		double_temp_y=(double)(double_current_y-double_scan_y);//current_scan cause current us larger!!!
		}

		slope=double_temp_y/double_temp_x;


	
		angle=atan(slope);

		y_double_freq=(double)(xy_max_freq*sin(angle));
		x_double_freq=(double)(xy_max_freq*cos(angle));

		
		y_freqvar=(int)(2000000/y_double_freq);
		x_freqvar=(int)(2000000/x_double_freq);

		OCR3A=x_freqvar;
		OCR4A=y_freqvar;


		target_x=(long)((double_temp_x)*80.000);//might be wrong, was double scan
		target_y=(long)((double_temp_y)*80.000);
		
		xroll=1;
		yroll=1;

			if(double_scan_e>0)//SETTING THE REQUIRED EXTRUSION SPEED
			{
				if(double_temp_y>double_temp_x)
				{
				xy_double_length=double_temp_x/cos(angle);
				}else
				xy_double_length=double_temp_y/sin(angle);
			
			e_time=xy_double_length/(xy_max_freq/80);
			e_double_freq=(double_scan_e*1000)/e_time;
			e_freqvar=(int)(2000000/e_double_freq);
			target_e=(long)(double_scan_e*1000);			
			
			OCR5A=e_freqvar;			

			edir=1;
			eroll=1;
			
			}

	}//machination with directions would be nice and also y=0 x=0 cases while avoiding division by 0
	else if(double_scan_x>0&&double_scan_y==0)
	{
		if(double_scan_x>double_current_x)
		{
		double_temp_x=double_scan_x-double_current_x;
		xdir=0;
		}

		if(double_scan_x<double_current_x)
		{
		double_temp_x=double_current_x-double_scan_x;
		xdir=1;
		}

		target_x=(long)(double_temp_x*80);
		target_y=(long)current_y;
		
		x_freqvar=2000000/xy_max_freq;
		OCR3A=x_freqvar;

		xroll=1;
		yroll=0;	
		
			if(double_scan_e>0)
			{
			e_time=target_x/xy_max_freq;
			e_double_freq=(double_scan_e*1000)/e_time;
			e_freqvar=(int)(2000000/e_double_freq);
			target_e=(long)(double_scan_e*1000);			
			
			OCR5A=e_freqvar;			

			edir=1;
			eroll=1;
			}
		mode=2;

	}
	else if(double_scan_x==0&&double_scan_y>0)
	{
		if(double_scan_y>double_current_y)
		{
		double_temp_y=double_scan_y-double_current_y;
		ydir=0;
		}

		if(double_scan_y<double_current_y)
		{
		double_temp_y=double_current_y-double_scan_y;
		ydir=1;
		}

		target_y=(long)(double_temp_y*80);
		target_x=current_x;
		
		y_freqvar=2000000/xy_max_freq;
		OCR3A=y_freqvar;

		xroll=0;
		yroll=1;	
		
			if(double_scan_e>0)
			{
			e_time=target_x/xy_max_freq;
			e_double_freq=(double_scan_e*1000)/e_time;
			e_freqvar=(int)(2000000/e_double_freq);
			target_e=(long)(double_scan_e*1000);			
			
			OCR5A=e_freqvar;			

			edir=1;
			eroll=1;
			}

	}
	else if(double_scan_x==0&&double_scan_y==0)
	{
	target_x=(long)current_x;
	target_y=(long)current_y;
	xroll=0;
	yroll=0;
	eroll=0;
	}


	
	
	sei();


}

















int main(int argc, char *argv[])
{
int i=0;
int h=0;
int bupi=0;


cli();
memset(command,'$',58);
init();
timer0_init();
timer1_init();
timer3_init();
timer4_init();
timer5_init();
//lcd_init();
//lcd_string("Welcome");
//lcd_move(0,1);
//lcdconfig();
//lcd_string("Give me a code:");
//lcd_move(0,2);
UART_init();
//adc_init();
//lcdsenddata('F');
//lcd_num(123456);
_delay_ms(1500);
sei();

	while(1)
	{
//-------------------------------------------------scan for input	
		while(mode==0)
		{
			while(1)//scan line
			{				

				if(command[comm]=USART_Receive())
				{
					if((command[comm]==13||command[comm]=='\n'||command[comm]==59)&&(comm<59))
					{
					break;
					}

					if(echo==1)
					{
					//USART_Transmit('[');
				//	USART_Transmit(comm+48);
				//	USART_Transmit(']');
					USART_Transmit(command[comm]);
					}

					comm++;

				}
				else
				{
				}

			
			}
			mode=1;
		
			if(echo==1)
			{
			USART_String_Transmit("\nTransition to parsing.");
			}
		
		}
		


//-------------------------------------------------parse commands
		while(mode==1)//parse commands
		{
			for(i=0;i<comm;i++)//cycling i till \n-1
			{

				if(command[i]=='N'&&command[i+1]>47&&command[i+1]<58)//GET LINE NUMBER
				{
					if(echo)
					{
					USART_String_Transmit("\n Getting line number.");
					}

					temp_line_number=fetchlong(command,i,'N');
				}				

				if(command[i]=='M'&&command[i+1]=='1'&&command[i+2]=='0'&&command[i+3]=='5')//GET EXTRUDER TEMP
				{

				}
				

				
				if(command[i]=='M'&&command[i+1]=='1'&&command[i+2]=='0'&&command[i+3]=='4')//SET EXTRUDER TEMP FAST SXXX
				{
	
				}					

				
				
				if(command[i]=='M'&&command[i+1]=='1'&&command[i+2]=='0'&&command[i+3]=='9')//SET EXTRUDER TEMP AND WAIT RXXX
				{
				
				}

				
				
				if(command[i]=='M'&&command[i+1]=='1'&&command[i+2]=='4'&&command[i+3]=='0')//SET BED TEMP FAST SXXX
				{
				
				}
				

				
				if(command[i]=='M'&&command[i+1]=='1'&&command[i+2]=='9'&&command[i+3]=='0')//WAIT FOR BED TEMP TO HIT RXXX
				{
				
				}

				
				
				if(command[i]=='M'&&command[i+1]=='1'&&command[i+2]=='0'&&command[i+3]=='7')//FAN OFF
				{
				
				}
				
				
				
				if(command[i]=='M'&&command[i+1]=='1'&&command[i+2]=='0'&&command[i+3]=='6')//FAN ON WITH FAN SPEED S255MAX
				{
				
				}

				
				

				if(command[i]=='M'&&command[i+1]=='8'&&command[i+2]=='2'&&command[i+3]==' ')//EXTRUDER TO ABSOLUTE MODE
				{
				
				}
				

				
				if(command[i]=='G'&&command[i+1]=='2'&&command[i+2]=='1'&&command[i+3]==' ')//METRIC VALUES
				{
				
				}

				
				
				if(command[i]=='G'&&command[i+1]=='9'&&command[i+2]=='0'&&command[i+3]==' ')//ABSOLUTE POSITIONING
				{
				
				}

				
				
				if(command[i]=='G'&&command[i+1]=='9'&&command[i+2]=='2'&&command[i+3]==' ')//ZERO EXTRUDED LENGTH WITH E0 otherwise set origin?
				{
				
				}

				
				
				if(command[i]=='G'&&command[i+1]=='0'&&command[i+2]==' ')//MOVE WITHOUT EXTUSION
				{
				if(echo)
				{
				USART_String_Transmit("\nG0 command.");
				}
				line_engine(double_scan_x,double_scan_y,double_scan_z,double_scan_e,i);
				if((echo==1)&&(xroll==1)&&(yroll=1))
				{
				USART_String_Transmit("\n Xroll=1");
				USART_String_Transmit("\n Yroll=1");
				}
				mode==2;
				
				}

				
				
				if(command[i]=='G'&&command[i+1]=='1'&&command[i+2]==' ')//MOVE WITH EXTRUSION
				{



				}

				
				
				if(command[i]=='Q'&&command[i+1]=='1')//MOTOR TEST					{
				{
					if(echo)
					{
					USART_String_Transmit("\n Motor test on.");
					//xroll=1;
					yroll=1;
					//testval=(threefetchdouble(command,i,'E')/0.01)*16;
					}
				}

				
				
				if(command[i]=='Q'&&command[i+1]=='0')//MOTOR TEST
				{
					if(echo)
					{
					USART_String_Transmit("\n Motor test off.");
					}
					//xroll=0;
					yroll=0;
				}
				

				
			}
			mode=2;
				if(echo)
				{
				USART_String_Transmit("\nTransition to set outputs.");
				}
			


		}	



//-------------------------------------------------setting outputs
		while(mode==2)
		{
			while(zroll==1||eroll==1||xroll==1||yroll==1)
			{
				if(USART_Receive()=='.')
				{
				zroll=0;
				eroll=0;
				xroll=0;
				yroll=0;
				
				USART_String_Transmit("\nSW Emergency stopped, please reset!");
			//	PINK|=0b00000001;
			//	PINA|=0b00000100;
			//	PINF|=0b00000100;
			//	PIND|=0b10000000;

				}
			}

			if(zroll==0&&eroll==0&&xroll==0&&yroll==0)
			{
				if(echo)
				{
				USART_String_Transmit("\nTransition to mode 3, reseting parameters.");
				}

				mode=3;
			}


		}



//-------------------------------------------------clear things
		while(mode==3)
		{
		cli();

		memset(command,'$',58);
		comm=0;
		i=0;
		double_scan_x=-1.0;
		double_scan_y=-1.0;
		double_scan_z=-1.0;
		double_scan_e=-1.0;
		
			if(echo)
			{
			USART_String_Transmit("\nParameters reseted returning to scanning mode");
			}

		USART_String_Transmit("\n\rok");
		
		sei();

		mode=0;
		}



		
//-------------------------------------------------
		while(mode==4)
		{

		}		

	}



return 0;
}



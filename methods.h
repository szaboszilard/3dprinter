#ifndef methods_H_
#define methods_H_

void init();
void led_out(int led);
void my_led_out(int led);
int G_port_button();
void Segment_Place(int place,int number);
void Segment_Write(int number);
void My_Segment_Write(int number,int del);
void rgb_led_out(int input);

#define SPI_DDR DDRB //spi direction
#define SPI_PORT PORTB//spi port
#define SPI_SCK PB0	//
#define SPI_MOSI PB1	//
#define SPI_MISO PB2	//

#define SPI_CS PB4
#define SPI_SS PB0
#define CS_EN (SPI_PORT &=~(1<<SPI_CS))
#define CS_DTS (SPI_PORT |=(1<<SPI_CS))
#define MASTER 0
#define SLAVE 1

int digit;

int read_out();
void timer0_init();
void timer1_init();
void timer3_init();
void timer4_init();
void timer5_init();
void lcd_init();

void lcdsenddata(int c);
void lcdsendcommand(int c);
void clock();

void lcdstring();
void mylcdstring();

void lcd_animated_char1(int start, int delay);

void Createmyobject00();
void selectmyobject(int digit);

void UART_init();
void USART_Transmit( unsigned char data );
unsigned char USART_Receive( void );

void USART_String_Transmit( char* pointer );


void my_lcd_string(char *pointer,int line, int pos);
void my_lcd_numbers(uint16_t number,int line, int pos);
void lcd_string(char *pointer);
void lcd_move(int column, int row);
void lcd_num(unsigned long number);
void buzzer_on();

void adc_init();
int ADC_read(int ch);

void SPI_init(int select);

int fetchint(char *t,int p, int ch);
long fetchlong(char *t,int p, int ch);
double threefetchdouble(char *t,int p, int ch);
double fourfetchdouble(char *t,int p, int ch);

#define DATA_DDR DDRE // data direction
#define DATA_PORT PORTE //data port
#define COMM_DDR DDRF 	//command direction
#define COMM_PORT PORTF //command port
//E if 1 puts the data on rising edge, on falling you can see
// RW reads and writes? 
//RS 0-data hex 'A' hex 'B' 1- command like 0x01 clear data, 0x28 new line start blinking

void matrix_to_led(int matrix);
void UART_cmd();

#define CMD_RS PF1//instruction register . IF COMMAND -->0 ,IF DATA -->1
#define CMD_RW PF2//read or write if write-->0 if read --1
#define CMD_EN PF3//enable


#endif

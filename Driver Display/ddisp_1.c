#include <18F26K80.h>
#device adc=8

#FUSES NOWDT                    //No Watch Dog Timer
#FUSES NOXINST                  //Extended set extension and Indexed Addressing mode disabled (Legacy mode)
#FUSES LP                       //Low power osc < 200 khz
#FUSES BROWNOUT
#FUSES HSH                      //High speed Osc, high power 16MHz-25MHz
#FUSES NOPLLEN                  //4X HW PLL disabled, 4X PLL enabled in software
#FUSES SOSC_DIG                 //Digital mode, I/O port functionality of RC0 and RC1    
#FUSES PUT
#FUSES NOIESO
#FUSES NOFCMEN
#FUSES NOPROTECT

#use delay(clock=20000000)
#USE SPI (MASTER, CLK=PIN_C3, DI=PIN_C5, DO=PIN_C4, MODE=0, BITS=8, STREAM=SPI_1, MSB_FIRST)

//Defines
#define LE PIN_C0
#define OE PIN_C1
#define BUZZER PIN_C2
//#define LED PIN_A5

//LED Driver Bytes
int Driver1[]= {0x00, 0x00};//other lights,Left bar
int Driver2[]= {0x00, 0x00};//Right bar graph,other lights
int Driver3[]= {0x00, 0x00};//Right seg,Left seg
int BarL[] = {0x00,0b00000001,0b00000011,0b00000111,0b00001111,0b00011111,0b00111111,0b01111111,0b11111111};
int BarR[] = {0x00,0b10000000,0b11000000,0b11100000,0b11110000,0b11111000,0b11111100,0b11111110,0b11111111};
int SegL[]={0xEE,0x82,0xDC,0xD6,0xB2,0x76,0x7E,0xC2,0xFE,0xF6,0b01111100, 0b10111010};//0-9,Error,HI
int SegR[]={0xEE,0x28,0xCD,0x6D,0x2B,0x67,0xE7,0x2C,0xEF,0x6F,0b10000110,0x28};//0-9,Error,HI

int8 dial;


void Update(signed int8 data);//write speed prototype
void flash(signed int8 data); //flash LEDs prototype

void main()
{
   signed int8 i = 0;//test variable
   setup_adc(ADC_CLOCK_DIV_32);
   setup_adc_ports(sAN10|VSS_VDD);
   set_adc_channel(10);
   delay_us(10);
   
   setup_timer_4(T4_DISABLED,0,1);
   setup_comparator(NC_NC_NC_NC);// This device COMP currently not supported by the PICWizard
   setup_spi(SPI_MASTER|SPI_SCK_IDLE_LOW|SPI_XMIT_L_TO_H|SPI_CLK_DIV_16|SPI_SAMPLE_AT_MIDDLE);
   
   
while(1){
   //set_adc_channel(10);
   //delay_us(10);
   dial = read_adc(); //pot2 is 16bit [0,65535]
   //dial=dial/65535*100;
   
   output_low(OE);
   output_low(LE);
   
  //for(i<110;i++;){//test
      flash(dial);
      //delay_ms(200);
   //}

//output_high(LED);
//delay_ms(500);
//output_low(LED);
//delay_ms(500);
// testing the buzzer
/*output_high(Buzz);
delay_ms(500);
output_low(buzz);
delay_ms(500);
*/
}


}


void Update(signed int8 data){
int msb=0;
int lsb=0;

int leftB = 1;
int rightB = 1;
int BSDial = data>>5;


if(data < 0){//speed less than 0 error
   msb = 10;//displays E
   lsb = 10;//displays r
}
if(data>=100){//fix double digit spillover
   //data=data-100;//display as a two digit number
   
   msb = 11;//displays H
   lsb = 11;//displays i
}
else {//seperate digits
   msb = data/10;
   lsb = (int)(data-msb*10);
}


//SPI writes to the drivers
spi_write(0x00);//misc lights
spi_write(BarL[BSDial]);//BarL
spi_write(BarR[BSDial]);//BarR
spi_write(0x00);//misc lights
spi_write(SegR[lsb]);//Right Seg
spi_write(SegL[msb]);//Left Seg

output_high(LE);
delay_ms(1);
output_low(LE);


}





void flash(signed int8 data){

int msb=0;
int lsb=0;


int BSDial = data>>5;

if(data < 0){//speed less than 0 error
   msb = 10;//displays E
   lsb = 10;//displays r
}
if(data>=100){//fix double digit spillover
   //data=data-100;//display as a two digit number
   
   msb = 11;//displays H
   lsb = 11;//displays i
}
else {//seperate digits
   msb = data/10;
   lsb = (int)(data-msb*10);
}


spi_write(0x00);//misc lights
spi_write(BarL[8]);//BarL
spi_write(BarR[8]);//BarR
spi_write(0x00);//misc lights
spi_write(SegR[lsb]);//Right Seg
spi_write(SegL[msb]);//Left Seg

output_high(LE);
delay_ms(1);
output_low(LE);

delay_ms(500);

spi_write(0x00);//misc lights
spi_write(BarL[0]);//BarL
spi_write(BarR[0]);//BarR
spi_write(0x00);//misc lights
spi_write(SegR[lsb]);//Right Seg
spi_write(SegL[msb]);//Left Seg

output_high(LE);
delay_ms(1);
output_low(LE);

delay_ms(250);
}






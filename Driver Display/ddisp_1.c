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

// Typedefs
typedef unsigned int8 uint8;
typedef unsigned int16 uint16;

/*
 * 7 segment display bit mapping:
 * ex: bit 2 corresponds to 0x04
 *
 *   6666
 *  5    7
 *  5    7
 *  5    7
 *  5    7
 *   4444
 *  3    1
 *  3    1
 *  3    1
 *  3    1
 *   2222  0
 *
 * Note that the bytes on the right
 *
 */

//LED Driver Bytes
int driver1[] = {0x00, 0x00};//other lights,Left bar
int driver2[] = {0x00, 0x00};//Right bar graph,other lights
int driver3[] = {0x00, 0x00};//Right seg,Left seg
int barL[] = {0x00,0x01,0x03,0x07,0x0F,0x1F,0x3F,0x7F,0xFF};
int barR[] = {0x00,0x80,0xC0,0xE0,0xF0,0xF8,0xFC,0xFE,0xFF};
int segment[] = {0xEE,0x82,0xDC,0xD6,0xB2,0x76,0x7E,0xC2,0xFE,0xF6, 0x2C, 0xBA}; //0-9,L,H
int segChar[] = {0x2C,0xEE,0xBA,0x82}; // L, O, H, I

int8 dial;


void update(signed int8 data);//write speed prototype
void flash(signed int8 data); //flash LEDs prototype

void main() {
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
        
        flash(dial);
        
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


void update(signed int8 data){
    int msb=0;
    int lsb=0;
    
    int leftB = 1;
    int rightB = 1;
    int BSDial = data>>5;
    
    
    if(data < 0) {  //speed less than 0 error
        msb = 10;   //displays E
        lsb = 10;   //displays r
    }
    else if(data>=100){  //fix double digit spillover
        //data=data-100;//display as a two digit number
        
        msb = 11;   //displays H
        lsb = 11;   //displays i
    }
    else {          //seperate digits
        msb = data/10;
        lsb = (int)(data-msb*10);
    }
    
    
    //SPI writes to the drivers
    spi_write(0x00);//misc lights
    spi_write(barL[BSDial]);//barL
    spi_write(barR[BSDial]);//barR
    spi_write(0x00);//misc lights
    spi_write(segR[lsb]);//Right Seg
    spi_write(segL[msb]);//Left Seg
    
    output_high(LE);
    delay_ms(1);
    output_low(LE);
}



void flash() {
    int msb=0;
    int lsb=0;
    
    if(data < 0){           // speed less than 0 error
        msb = 10;           // displays E
        lsb = 10;           // displays r
    }
    else if(data>=100) {    // fix double digit spillover
        msb = 11;           // displays H
        lsb = 11;           // displays i
    }
    else {                  // seperate digits
        msb = data/10;
        lsb = data%10;
    }
    
    
    spi_write(0x00);        // Misc lights
    spi_write(barL[8]);     // barL
    spi_write(barR[8]);     // barR
    spi_write(0x00);        // Misc lights
    spi_write(segR[lsb]);   // Right Seg
    spi_write(segL[msb]);   // Left Seg
    
    output_high(LE);
    delay_ms(1);
    output_low(LE);
    
    delay_ms(500);
    
    spi_write(0x00);//misc lights
    spi_write(barL[0]);//barL
    spi_write(barR[0]);//barR
    spi_write(0x00);//misc lights
    spi_write(segR[lsb]);//Right Seg
    spi_write(segL[msb]);//Left Seg
    
    output_high(LE);
    delay_ms(1);
    output_low(LE);
    
    delay_ms(250);
}

/*  writeDisplay()
 *
 *  barL    - number between 0 and 8 inclusive
 *  barR    - number between 0 and 8 inclusive
 *  misc1   -
 *  misc2   -
 *  seg     -
 */
void writeDisplay(uint8 barL, uint8 barR, uint8 misc1, uint8 misc2, uint8 seg) {
    

    spi_write(0x00);        // Misc lights
    spi_write(barL[8]);     // barL
    spi_write(barR[8]);     // barR
    spi_write(0x00);        // Misc lights
    spi_write(segR[lsb]);   // Right Seg
    spi_write(segL[msb]);   // Left Seg
    
    output_high(LE);
    delay_ms(1);
    output_low(LE);
}


uint8 swapNibble(uint8 a) {
    return (a << 4) | ((a >> 4) & 0x0F);
}





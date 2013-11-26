//#include <i2cwhatever.h> //replaced by following lines of code

#include <16F883.h>
#device adc=16

#define CMND_RD_VOLT 0x0C
#define CMND_RD_TEMP 0x18

#define I2C_CMND_RD 1
#define I2C_CMND_WR 0


#FUSES NOWDT                    //No Watch Dog Timer
#FUSES INTRC                    //Internal Oscillator
#FUSES NOBROWNOUT               //No brownout reset
#FUSES NOLVP                    //No low voltage prgming, B3(PIC16) or B5(PIC18) used for I/O
#define LED PIN_A1

#use delay(clock=8000000)
#use rs232(baud=19200,parity=N,xmit=PIN_C6,rcv=PIN_C7,bits=8,stream=PORT1)
#use i2c(master,fast,sda=PIN_C4,scl = PIN_C3)

void main()
{

   unsigned int16 volts;
   unsigned int8 temp0;
   unsigned int8 temp1;
   unsigned int8 addr = 0x06; // slave device address
   
   setup_comparator(NC_NC_NC_NC);// This device COMP currently not supported by the PICWizard
   setup_oscillator(OSC_8MHZ);   //
   
   while(1){
      //led blinking
      output_high(LED);
      delay_ms(100);
      
      
      i2c_start();
      
      i2c_write((addr<<1) + I2C_CMND_WR); //write = 0
      i2c_write(CMND_RD_VOLT);
      
      i2c_start();   //change in data flow direction
      i2c_write((addr << 1) + I2C_CMND_RD); //read = 1
      temp0 = i2c_read(0);
      temp1 = i2c_read(0);
      i2c_stop();
      
      volts = (((unsigned int16)(temp0) & 0x7F) << 3) + (((unsigned int16)(temp1) >> 5) & 0x07); //0x07
      
      
      //putc
      printf("one: %02X, two: %02X\n\r", temp0, temp1);
      printf("Volt = %ld\n\r", volts); //in this context, int16 is long
      output_low(LED);
      delay_ms(100);
   }
}

#include <18F26K80.h>
#device adc=16

#define I2C_CMND_RD 1
#define I2C_CMND_WR 0

#FUSES NOWDT                  //No Watch Dog Timer
#FUSES HSH                //Internal Oscillator
#FUSES NOPLLEN
#FUSES NOBROWNOUT               //No brownout reset
//#FUSES NOLVP                    //No low voltage prgming, B3(PIC16) or B5(PIC18) used for I/O
#define LED PIN_A2

#use delay(clock=20000000)
#use rs232(baud=9600,xmit=PIN_C6,rcv=PIN_C7,parity=N,stream=debugs)
#use i2c(master,sda=PIN_C4,scl = PIN_C3)

void main()
{

   unsigned int8 slaveID;
   unsigned int16 volts;
   unsigned int16 temperature;
   int8 temp0;
   int8 temp1;
   int8 temp2;
   int8 temp3;
   unsigned int8 addr = 0x05; //all represent address, except last bit, which is read(0)/write(1) <- varies depending on datasheet for chip

   setup_comparator(NC_NC_NC_NC);// This device COMP currently not supported by the PICWizard
   //setup_oscillator(OSC_16MHZ);
    
   while(1){
      //led blinking
      output_high(LED);
      output_high(PIN_C2);
      delay_ms(1000);
      
      slaveID = 0x05; //slaveID HERE
      slaveID<<=1;
      
      i2c_start();    
         i2c_write(slaveID); //select slave, write bit
         i2c_write(0x0C); //register to read      
      i2c_start();   //change in data flow direction
         i2c_write(slaveID+1); //select slave, read bit
         temp0 = i2c_read(0);
      i2c_stop();
      
      //delay_ms(1);
      
      i2c_start();    
         i2c_write(slaveID); //select slave, write bit
         i2c_write(0x0D); //register to read  
      i2c_start();   //change in data flow direction
         i2c_write(slaveID+1); //select slave, read bit
         temp1 = i2c_read(0);
      i2c_stop();
      
      i2c_start();    
         i2c_write(slaveID); //select slave, write bit
         i2c_write(0x18); //register to read  
      i2c_start();   //change in data flow direction
         i2c_write(slaveID+1); //select slave, read bit
         temp2 = i2c_read(0);
      i2c_stop();
      
      i2c_start();    
         i2c_write(slaveID); //select slave, write bit
         i2c_write(0x19); //register to read  
      i2c_start();   //change in data flow direction
         i2c_write(slaveID+1); //select slave, read bit
         temp3 = i2c_read(0);
      i2c_stop();
      
      temp1 = temp1>>5;
      volts = temp0;
      volts = volts<<3;
      volts = volts + temp1;
      
      temp3 = temp3>>5;
      temperature = temp2;
      temperature = temperature<<3;
      temperature = temperature + temp3;
      
      putc(temp0);
      putc(temp1);
      putc(temp2);
      putc(temp3);
      putc(0xFF);
     
      float newVolts = volts/1024.0*5.0;
      printf("Volts: %.3f ", newVolts);
      
      float newTemperature = temperature/1024.0*127;
      printf("Temperature: %.3f\n\r", newTemperature);
      
      output_low(LED);
      output_low(PIN_C2);
      delay_ms(1000);
   }
}





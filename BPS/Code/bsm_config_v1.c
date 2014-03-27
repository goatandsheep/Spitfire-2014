/****************************************************************************
 * prot5_bsm_config_v1.c
 * ---------------------
 *
 * Developers: James Strack
 *
 * Current editors: James Strack
 *
 * McMaster Solar Car Project, 2009-2014
 *
 * COMPILER: CCS PCWHD 4.120
 *
 * DESCRIPTION:
 * ------------
 * - Used to reconfigure a single PROT5 battery side module (based on DS2764
 *   LiPo battery monitor IC)
 * - Performs a search over all possible DS2764 addresses and locates first 
 *   address which returns a valid temperature
 * - Prompts user (in serial monitor) to enter desired new module address
 * - Sets new address and enables power save mode
 * - Performs search again to verify that the new address has been applied
 *   successfully 
 * - LED1 is "ON" when BSM relay is "CLOSED" (battery connected to DS2764)
 * - LED1 is "OFF" when BSM relay is "OPEN" (battery disconnected from DS2764)
 * - LED2 is flashing when code finishes or error has occurred 
 *
 * TESTED WITH:
 * ------------
 * - PIC18F2458
 *
 * CURRENTLY IMPLEMENTED:
 * ----------------------
 * - Performs tasks outlined in description
 * - "lockup()" routine is entered following error or successful operation
 *
 * CODED BUT NOT TESTED:
 * --------------------
 * - N/A
 *
 * FUTURE WORK / BUGS:
 * -------------------
 *
 * REVISION LOG
 * ------------
 * 26-MAR-2014: Created / based on old code for PROT4 configuration tool
 * 
 ****************************************************************************/

#include <18F2458.h>
#device adc=12
#include <stdlib.h>

#define LED1            pin_a0
#define LED2            pin_a1
#define RELAY           pin_b4
#define VOLT_MSB_ADDR   0x0C
#define TEMP_MSB_ADDR   0x18
#define STATUS_REG      0x31
#define SLAVE_ADDRESS   0x32
#define SPECIAL         0x08
#define FN_CMD          0xFE //DS2764 function command
#byte   RCSTA =         0xFAB

//USB module control registers (see PIC18F2455/2458 datasheet)
#byte UCON=0xF6D
#byte UCFG=0xF6F

#FUSES NOWDT                    //No Watch Dog Timer
#FUSES WDT128                   //Watch Dog Timer uses 1:128 Postscale
#FUSES PLL1                     //No PLL PreScaler
#FUSES INTEC_IO                 //Internal Clock, EC used by USB, I/O on RA6
#FUSES NOBROWNOUT               //No brownout reset
#FUSES NOVREGEN                 //USB voltage regulator disabled
#FUSES NOLVP                    //No low voltage prgming, B3(PIC16) or B5(PIC18) used for I/O
#FUSES NOXINST                  //Extended set extension and Indexed Addressing mode disabled (Legacy mode)

#use delay(clock=8000000)
#use rs232(baud=19200,parity=N,xmit=PIN_C6,rcv=PIN_C7,bits=8,stream=PORT1)
#use i2c(Master,Fast,sda=PIN_B0,scl=PIN_B1)

short set_addr(unsigned int8 addr1, unsigned int8 addr2);
short set_powersave(unsigned int8 addr);
int8 get_temp(unsigned int8 addr);
void bsm_on(void);
void bsm_off(void);
int8 addr_search(void);
void lockup(void);

void main()
{

   int i;
   short read=1;
   short status=0;
   char input[7];
   unsigned int8 addr1=0;
   unsigned int8 addr2=0;
   unsigned int8 test;
   
   setup_oscillator(OSC_8MHZ|OSC_INTRC|OSC_31250|OSC_PLL_OFF);
   
   printf("\r\n\nMcMaster Solar Car Project");
   printf("\r\nPROT5 Module Configuration Tool");
   printf("\r\nF/W version 0.1, 26 March 2014\r\n");
   output_high(LED1);
   output_high(LED2);
   output_low(RELAY);
   delay_ms(500);
   output_low(LED1);
   output_low(LED2);
   bsm_on();
      
   printf("\r\nSearching for connected module...\r\n");
   addr1=addr_search();
   
   if(addr1==0xFF){
      printf("ERROR! No module found!\r\n");
      printf("Check connections and press reset!\r\n");
      lockup();
   }
   
   printf("Module found! Target address is %X\r\n",addr1);
   printf("\r\nEnter new target address in HEX: ");
   
   //Clear receive and storage buffer
   for(i=0;i<10;i++)if(kbhit())getc();
   for(i=0;i<7;i++)input[i]=0;
   input[0]="0";
   input[1]="x";
   bit_clear(RCSTA,4);    // clear the error caused by receive buffer over run
   bit_set(RCSTA,4);      // reset it back to working mode for receiver
   
   read=1;
   for(i=0;i<5 && read;i++){
      input[i+2]=getc();
      putc(input[i+2]);
      if (input[i+2]==0x0D){
         read=0;
         input[i+2]=0;
      }
   }
   addr2=atoi(input);
   printf("\r\nNew target address is %X",addr2);
   //Turn on module!
   bsm_on();
   
   printf("\r\nSetting address...");
   status = set_addr(addr1,addr2);
   
   if(status) {
      printf("\r\nProgramming not successful!");;
      putc(0x0D);
      putc(0x0A);
   }else{
      printf("\r\nSuccess!");
      putc(0x0D);
      putc(0x0A); 
   }

   printf("\r\nEnabling power save mode...");
   status = set_powersave(addr2);
   
   if(status) {
      printf("\r\nProgramming not successful!");
      putc(0x0D);
      putc(0x0A);
   }else{
      printf("\r\nSuccess!");
      putc(0x0D);
      putc(0x0A);  
   }
   
   test=addr_search();
   if(test==0xFF){
      printf("\r\nAn ERROR has occurred!\r\n");
      printf("Check above messages and press reset!\r\n");
      lockup();
   }
   
   printf("\r\nOperation complete! New address is %X",test);
   printf("\r\nTaking reading on new address:");
   printf("\r\nTemperature = %d\r\n",get_temp(test));
   printf("\r\nConnect next module and press reset!");
   putc(0x0D);
   putc(0x0A);
   bsm_off();
   lockup();   

}

short set_addr(unsigned int8 addr1,addr2){
   short ack;
   byte spc_feat=0;

   addr1<<=1;
   addr2<<=1;
    
   //Set "slave address write enable" bit in special register
   i2c_start();
   ack=i2c_write(addr1);
   i2c_write(SPECIAL);
   i2c_start();
   i2c_write(addr1+1);
   spc_feat = i2c_read(0);
   i2c_stop();
   
   if(!ack){
   
   bit_set(spc_feat,1);
   
   i2c_start();
   i2c_write(addr1);
   i2c_write(SPECIAL);
   i2c_write(spc_feat);
   i2c_stop();
   
   //Send new address to slave
   i2c_start();
   i2c_write(addr1);
   i2c_write(SLAVE_ADDRESS);
   i2c_write(addr2);
   i2c_stop();
   
   //Clear "slave address write enable" bit in special register
   bit_clear(spc_feat,1);
   
   i2c_start();
   i2c_write(addr2);
   i2c_write(SPECIAL);
   ack = i2c_write(spc_feat);
   i2c_stop();
   
   //Copy address shadow RAM to EEPROM
   i2c_start();
   i2c_write(addr2);
   i2c_write(FN_CMD);
   ack = i2c_write(0x44);
   i2c_stop();
   delay_ms(10); //wait for command to finish
   }
   
   return(ack);
}

short set_powersave(unsigned int8 addr){
   short ack;
   byte stat=0;

   addr<<=1;
    
   //Set "power save" bit in status register
   i2c_start();
   i2c_write(addr);
   i2c_write(STATUS_REG);
   i2c_start();
   i2c_write(addr+1);
   stat = i2c_read(0);
   i2c_stop();
   
   bit_set(stat,5);
   
   i2c_start();
   i2c_write(addr);
   i2c_write(STATUS_REG);
   i2c_write(stat);
   i2c_stop();
     
   //Copy address shadow RAM to EEPROM
   i2c_start();
   i2c_write(addr);
   i2c_write(FN_CMD);
   ack = i2c_write(0x44);
   i2c_stop();
   delay_ms(10); //wait for command to finish 
   
   return(ack);
}

int8 get_temp(unsigned int8 addr){

    int8 temp_msb;

    addr<<=1;
    i2c_start();
    i2c_write(addr);
    i2c_write(TEMP_MSB_ADDR);
    i2c_start();
    i2c_write(addr+1); // R/W bit now = 1
    temp_msb=i2c_read(0);
    i2c_stop();

    return(temp_msb);
}

void bsm_on(void){
   output_high(LED1);
   output_high(RELAY);
   //Wait for contact to settle
   delay_ms(100);
}

void bsm_off(void){
   output_low(LED1);
   output_low(RELAY);
   //Wait for contact to settle
   delay_ms(100);
}

int8 addr_search(void){
   int8 i,temp;
   unsigned int8 addr=0xFF;
     
   //Turn on battery side module
   bsm_on();
   
   //Begin search
   
   for(i=0;i<=0x7F;i++){
      temp=get_temp(i);
      if(temp<0xFF){
      addr=i;
      i=0xF0;
      }
   }
   return(addr);

}

void lockup(){
   // Function to lockup execution if code has finished or an error has occurred
   // Turn off relay to disconnect BSM if this hasn't been done
   bsm_off();
   while(true){
      output_toggle(LED2);
      delay_ms(500);
   }
}

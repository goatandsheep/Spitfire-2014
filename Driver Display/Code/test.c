#include <18F26K80.h>
#device adc=12

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

#define LE PIN_C0
#define OE PIN_C1
#define Buzzer PIN_C2

void main()
{

   setup_adc_ports(sAN0);
   setup_timer_4(T4_DISABLED,0,1);
   setup_comparator(NC_NC_NC_NC);// This device COMP currently not supported by the PICWizard
   setup_spi(SPI_MASTER|SPI_SCK_IDLE_LOW|SPI_XMIT_L_TO_H|SPI_CLK_DIV_16|SPI_SAMPLE_AT_MIDDLE);


while(1){
output_low(OE);
output_low(LE);
spi_write(0b11111111);
spi_write(0b11111111);
spi_write(0b11111111);
spi_write(0b11111111);

output_high(LE);
delay_ms(1);
output_low(LE);
delay_ms(300);
// testing the buzzer
/*output_high(Buzz);
delay_ms(500);
output_low(buzz);
delay_ms(500);*/

}


}










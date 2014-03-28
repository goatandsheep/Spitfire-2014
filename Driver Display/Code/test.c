#include <18F26K80.h>

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

void main() {
    delay_ms(1000);
    printf("hi\n");
}










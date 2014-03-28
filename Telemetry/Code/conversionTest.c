#include <18F26K80.h>

#FUSES NOWDT                    //No Watch Dog Timer
#FUSES SOSC_DIG                 //Digital mode, I/O port functionality of RC0 and RC1
#FUSES NOXINST                  //Extended set extension and Indexed Addressing mode disabled (Legacy mode)
#FUSES HSH                      //High speed Osc, high power 16MHz-25MHz
#FUSES NOPLLEN                  //4X HW PLL disabled, 4X PLL enabled in software
#FUSES BROWNOUT                 //Reset when brownout detected               
#FUSES PUT                      //Power Up Timer
#FUSES NOIESO                   //Internal External Switch Over mode disabled
#FUSES NOFCMEN                  //Fail-safe clock monitor disabled
#FUSES NOPROTECT                //Code not protected from reading

#use delay(clock = 20000000)
#use rs232(baud = 115200, parity = N, UART1, bits = 8, ERRORS)
#use i2c(master, sda = PIN_C4, scl = PIN_C3)
#define LED PIN_B5

typedef unsigned int8 uint8;
typedef unsigned int16 uint16;
typedef unsigned int32 uint32;

uint8 rawData[4] = {0x40, 0x4C, 0xCC, 0xCD};
int32 fullBits;
float realValue;

void main(void) {
    setup_timer_2(T2_DIV_BY_4,79,16);    //setup up timer2 to interrupt every 1ms if using 20Mhz clock
    enable_interrupts(INT_TIMER2);   //enable timer2 interrupt (if want to count ms)
    enable_interrupts(GLOBAL);       //enable all interrupts

    fullBits  = (int32)rawData[3]<<24;
    fullBits += (int32)rawData[2]<<16;
    fullBits += (int32)rawData[1]<<8;
    fullBits += (int32)rawData[0];
    
    while (1) {
    printf("start\r\n");
    //realValue = (float)fullBits;
    //printf("%f\r\n",realValue);
    /*memcpy(&fullBits, &realValue, 4);
    printf("%f\r\n",realValue);*/
    memcpy(rawData, &realValue, 4);
    printf("memcpy: %f\r\n",realValue);
    printf("cast:   %f\r\n",*(float*)&fullBits);
    printf("end\r\n");
    
    delay_ms(3000);
    }
} 

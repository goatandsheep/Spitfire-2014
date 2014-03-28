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

uint8 raw[4];
float32 oldF, newF;

void floatToRaw(uint8 *raw, float f) {
    memcpy(raw, &f, 4);
}

float rawToFloat(uint8 *raw) {
    float f;
    memcpy(&f, raw, 4);
    return f;
}

// RAW IS STORED IN MICROFLOAT
void IEEEFloatToRaw(uint8 *raw, float f) {
    int1 lsb;
    int1 sign;
    
    // Load raw binary
    memcpy(raw, &f, 4);
    
    // Swap sign and exponent
    IEEERawToRaw(raw);
}

void IEEERawToRaw(uint8 *raw) {
    int1 sign =  bit_test(raw[0], 7);
    int1 lsb = bit_test(raw[1], 7);
    raw[0] = (raw[0] << 1) | lsb;
    raw[1] = (raw[1]&0x7F) | (sign << 7);
}


void main(void) {
    setup_timer_2(T2_DIV_BY_4,79,16);   //setup up timer2 to interrupt every 1ms if using 20Mhz clock
    enable_interrupts(INT_TIMER2);      //enable timer2 interrupt (if want to count ms)
    
    delay_ms(5000);
    printf("\n\n\rSTART!\n\n\r");

    while(1) {
        //oldF = 123.45f;
        
        //*
        raw[0] = 0x42;
        raw[1] = 0xF6;
        raw[2] = 0xE6;
        raw[3] = 0x66;
        //*/
    
        printf("\r\n");
        
        printf("orig     bytes = {");
        printf("%02X ", raw[0]);
        printf("%02X ", raw[1]);
        printf("%02X ", raw[2]);
        printf("%02X}\r\n", raw[3]);
    
        //*
        memcpy(&oldF, raw, 4);
        //printf("float = %f\r\n", oldF);
        IEEEFloatToRaw(raw, oldF);
        //*/
        
        printf("memcpy'd bytes = {");
        printf("%02X ", raw[0]);
        printf("%02X ", raw[1]);
        printf("%02X ", raw[2]);
        printf("%02X}\r\n", raw[3]);
        
        newF = rawToFloat(raw);
        printf("memcpy'd from bytes = %f\r\n", newF);
        
        
        delay_ms(5000);
    }
}

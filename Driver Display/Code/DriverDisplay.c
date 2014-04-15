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
#use SPI(MASTER, CLK=PIN_C3, DI=PIN_C5, DO=PIN_C4, MODE=0, BITS=8, STREAM=SPI_1, MSB_FIRST)
#define LED PIN_C0
#define RTS PIN_C5

#define LE PIN_C0   // Latch Enable
#define OE PIN_C1   // Output Enable
#define BUZZER PIN_C2

#define MOTOR_CONT_ID 0x400
#define BUS_ID        MOTOR_CONT_ID + 2
#define VELOCITY_ID   MOTOR_CONT_ID + 3

//MPPT ID HERE

#define BPS_ID 0x800
#define BPS_ERROR BPS_ID + 5

// Modified CAN library includes default FIFO mode, timing settings match MPPT, 
// and 11-bit instead of 24-bit addressing
#include "../../Shared/Code/can-18F4580_mscp.c"
#include "../../Shared/Code/float_util.c"

typedef unsigned int8 uint8;
typedef unsigned int16 uint16;

void setup(void);
void getCANData(void);
void writeDisplay(uint8 lBarN, uint8 rBarN, int8 bottomLEDs[4], signed int8 num);
uint8 swapNibble(uint8 a);

// Driver LED bytes
int barL[] = {0x00,0x01,0x03,0x07,0x0F,0x1F,0x3F,0x7F,0xFF};
int barR[] = {0x00,0x80,0xC0,0xE0,0xF0,0xF8,0xFC,0xFE,0xFF};
int segment[] = {0xEE,0x82,0xDC,0xD6,0xB2,0x76,0x7E,0xC2,0xFE,0xF6}; // 0-9
int segChar[] = {0x2C,0xEE,0xBA,0x82}; // L, O, H, I

int8 dial;
float busCurrent;
float busVoltage;
float carSpeed;
int8 BPSWarn[4];

uint8 leftBar = 0, rightBar = 0;
int8 misc[4] = {1, 0, 1, 1};
uint8 segmentNum = 0;

void setup(void) {   
    setup_adc(ADC_CLOCK_DIV_32);
    setup_adc_ports(sAN10|VSS_VDD);
    set_adc_channel(10);
    delay_us(10);
    
    setup_timer_4(T4_DISABLED,0,1);
    setup_comparator(NC_NC_NC_NC);// This device COMP currently not supported by the PICWizard
    setup_spi(SPI_MASTER|SPI_SCK_IDLE_LOW|SPI_XMIT_L_TO_H|SPI_CLK_DIV_16|SPI_SAMPLE_AT_MIDDLE);
    
    can_init();
    set_tris_b((*0xF93 & 0xFB) | 0x08);  //b3 is out, b2 is in (default)
    
    //output_low(OE);
    //output_low(LE);
    //setup_ccp2(OE);  // Configure CCP2 as a PWM 
    //setup_power_pwm_pins(PWM_OFF, PWN_ON, PWM_OFF, PWM_OFF);
}

void main() {
    int i;
    setup();
    
    while(1){
        output_high(RTS);
        dial = read_adc(); //potentiometer is 8bit [0,255]
        
        
        output_low(OE);
        output_low(LE);
        //set_pwm2_duty(dial%100);
        //if (getCANData())
        //    writeDisplay(mpptIn, motorOut, BPSWarn, (int)carSpeed);
        
        writeDisplay(leftBar,rightBar,misc,segmentNum);
        segmentNum = ++segmentNum%10;
        
        for (i = 0; i < 100; i++) {
            output_low(OE);
            delay_us(100);
            output_high(OE);
            delay_us(3000);
        }
   }
}

int getCANData(void) {
    struct rx_stat rxstat;
    int32 rx_id;
    int8 in_data[8];
    int rx_len;
    
    // If there is no CAN message waiting in the queue, return
    if (!can_kbhit())
        return 0;
    
    // If data is waiting in buffer...
    if(can_getd(rx_id, in_data, rx_len, rxstat)) {
        switch(rx_id) {
        case BUS_ID:
            busCurrent = IEEERawToFloat(&in_data[0]);
            busVoltage = IEEERawToFloat(&in_data[4]);
            break;
        case VELOCITY_ID:
            carSpeed = IEEERawToFloat(&in_data[0]);
            carSpeed *= 3.6;    // convert m/s to km/h
            break;
        case BPS_ERROR:
            // See BPS/Documents/Documentation.txt
            BPSWarn[0] = in_data[0] | in_data[1]&0xFC;  // Over Voltage
            BPSWarn[1] = in_data[2] | in_data[3]&0xFC;  // Under Voltage
            BPSWarn[2] = in_data[4] | in_data[5]&0xFC;  // Over Temperature
            BPSWarn[3] = in_data[5]&0x01;               // Over Current
            break;
        }
    }
    
    return 1;
}

/*  writeDisplay()
 *
 *  barL       - number between 0 and 8 inclusive
 *  barR       - number between 0 and 8 inclusive
 *  bottomLEDs - four misc lights on bottom
 *  num        - number to be displayed on the 7+1 segment display
 */
void writeDisplay(uint8 lBarN, uint8 rBarN, int8 bottomLEDs[4], signed int8 num) {
    uint8 lSeg;
    uint8 rSeg;
    int8 bot = 0;
    bot |= ((bottomLEDs[0]&1) << 7);
    bot |= ((bottomLEDs[1]&1) << 6);
    bot |= ((bottomLEDs[2]&1) << 1);
    bot |=  (bottomLEDs[3]&1);
    
    if (num < 0) {
        lSeg = segChar[0];      // L
        rSeg = segChar[1];      // O
    } else if (num > 99) {
        lSeg = segChar[2];      // H
        rSeg = segChar[3];      // I
    } else {
        lSeg = segment[num / 10];
        rSeg = segment[num % 10];
    }

    
    spi_write(bot);                 // 2 Bottom Left LEDs
    spi_write(barL[lBarN]);         // Left Bar
    spi_write(barR[rBarN]);         // Right Bar
    spi_write(bot);                 // 2 Bottom Right LEDs
    spi_write(swapNibble(rSeg));    // Right Seg
    spi_write(lSeg);                // Left Seg
    
    // Enable latch momentarily then disable, why?
    output_high(LE);
    delay_ms(1);
    output_low(LE);
}

/*  swapNibble()
 *
 *  a - 8bit number to be swapped
 *
 *  Returns:
 *  Last four bits of a, then first four bits of a, in one byte 
 */
uint8 swapNibble(uint8 a) {
    return (a << 4) | ((a >> 4) & 0x0F);
}

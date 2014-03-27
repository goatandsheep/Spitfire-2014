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

//Defines
#define LE PIN_C0
#define OE PIN_C1
#define BUZZER PIN_C2

#define BPS_ID 0x800
#define BPS_VOLT1 BPS_ID + 1
#define BPS_VOLT2 BPS_ID + 2
#define BPS_TEMP1 BPS_ID + 3
#define BPS_TEMP2 BPS_ID + 4
#define BPS_ERROR BPS_ID + 5

#include <can-18F4580_mscp.c>  // Modified CAN library includes default FIFO mode, timing settings match MPPT, 11-bit instead of 24-bit addressing
// Typedefs
typedef unsigned int8 uint8;
typedef unsigned int16 uint16;

//LED Driver Bytes
int driver1[] = {0x00, 0x00};//other lights,Left bar
int driver2[] = {0x00, 0x00};//Right bar graph,other lights
int driver3[] = {0x00, 0x00};//Right seg,Left seg
int barL[] = {0x00,0x01,0x03,0x07,0x0F,0x1F,0x3F,0x7F,0xFF};
int barR[] = {0x00,0x80,0xC0,0xE0,0xF0,0xF8,0xFC,0xFE,0xFF};
int segment[] = {0xEE,0x82,0xDC,0xD6,0xB2,0x76,0x7E,0xC2,0xFE,0xF6}; // 0-9
int segChar[] = {0x2C,0xEE,0xBA,0x82}; // L, O, H, I

int8 dial;

int1 BPSWarn[4];

void main() {
    struct rx_stat rxstat;
    int32 rx_id;
    int8 in_data[8];
    int rx_len;
    
    setup_adc(ADC_CLOCK_DIV_32);
    setup_adc_ports(sAN10|VSS_VDD);
    set_adc_channel(10);
    delay_us(10);
    
    setup_timer_4(T4_DISABLED,0,1);
    setup_comparator(NC_NC_NC_NC);// This device COMP currently not supported by the PICWizard
    setup_spi(SPI_MASTER|SPI_SCK_IDLE_LOW|SPI_XMIT_L_TO_H|SPI_CLK_DIV_16|SPI_SAMPLE_AT_MIDDLE);
    
    can_init();
    set_tris_b((*0xF93 & 0xFB) | 0x08);  //b3 is out, b2 is in (default)
    
    while(1){

        output_high(RTS);
        dial = read_adc(); //pot2 is 16bit [0,65535]
        
        output_low(OE);
        output_low(LE);
        
        flash(dial);
        if (can_kbhit()) {
            // If data is waiting in buffer...
            if(can_getd(rx_id, in_data, rx_len, rxstat)) {
                
                if(rx_id == BPS_ERROR) {
                    setBPSWarn(in_data);
                }
                /** get data from buffer, and place it into the fields: rx_id, in_data, rx_len... etc
                printf("RECIEVED: BUFF=%U ID=%3LX LEN=%U OVF=%U ", rxstat.buffer, rx_id, rx_len, rxstat.err_ovfl);
                printf("FILT=%U RTR=%U EXT=%U INV=%U\r\n", rxstat.filthit, rxstat.rtr, rxstat.ext, rxstat.inv);
                printf("\tDATA = ");
                for (i=0;i<rx_len;i++)
                    printf("%02X ", in_data[i]); */
            } else {
                
            }           
        }                  
    }
}

/*  setBPSWarn()
 *
 *  in_data - array of ???
 *
 *  Lun pls comment why?
 */
void setBPSWarn(int8 in_data[6]) {
    BPSWarn[0] = in_data[0] | in_data[1]&0xFC;
    BPSWarn[1] = in_data[2] | in_data[3]&0xFC;
    BPSWarn[2] = in_data[4] | in_data[5]&0xFC;
    BPSWarn[3] = in_data[5]&0x01;
}

/*  writeDisplay()
 *
 *  barL    - number between 0 and 8 inclusive
 *  barR    - number between 0 and 8 inclusive
 *  misc1   - misc lights on top
 *  misc2   - misc lights on bottom
 *  num     - number to be displayed on the 7SD
 */
void writeDisplay(uint8 lBarN, uint8 rBarN, uint8 misc1, uint8 misc2, int8 num) {
    uint8 lSeg;
    uint8 rSeg;

    if (segNum < 0) {
        lSeg = segChar[0];      // L
        rSeg = segChar[1];      // O
    } else if (segNum > 99) {
        lSeg = segChar[2];      // H
        rSeg = segChar[3];      // I
    } else {
        lSeg = segment[num / 10];
        rSeg = segment[num % 10];
    }

    spi_write(misc1);               // Misc lights
    spi_write(barL[lBarN]);         // Left Bar
    spi_write(barR[rBarN]);         // Right Bar
    spi_write(misc2);               // Misc lights
    spi_write(swapNibble(rSeg));    // Right Seg
    spi_write(lSeg);                // Left Seg
    
    // What is this and why?
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





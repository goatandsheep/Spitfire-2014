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

#define MOTOR_CONT_ID 0x400
#define STATUS_ID         MOTOR_CONT_ID + 1
#define BUS_ID            MOTOR_CONT_ID + 2
#define VELOCITY_ID       MOTOR_CONT_ID + 3
#define IPM_MOTOR_TEMP_ID MOTOR_CONT_ID + 11
#define BOARD_TEMP_ID     MOTOR_CONT_ID + 12
#define ODOMETER_ID       MOTOR_CONT_ID + 14

#define BPS_ID 0x800
#define BPS_VOLT1 BPS_ID + 0
#define BPS_VOLT2 BPS_ID + 1
#define BPS_TEMP1 BPS_ID + 2
#define BPS_TEMP2 BPS_ID + 3
#define BPS_ERROR BPS_ID + 4

// Modified CAN library includes default FIFO mode, timing settings match MPPT, 
// and 11-bit instead of 24-bit addressing
#include "../../Shared/Code/can-18F4580_mscp.c"

// Typedefs
typedef unsigned int8 uint8;
typedef unsigned int16 uint16;
typedef unsigned int32 uint32;

uint16 ms = 0;

uint8 statusData[8];
uint8 busData[8];
uint8 motorData[4];
uint8 motorIPMTempData[8];
uint8 boardTempData[4];
uint8 odometerData[4];

uint8 BPSVolt[14];
uint8 BPSTemp[14];
uint8 BPSCurr[2];
uint8 BPSError[6];

void getData(void);
int8 getTemp(void);
void printData(void);

// Interrupt service routines
#int_timer2
void isr_timer2(void) {
    ms++;
}

void main(void) {
   uint8 tempswap;
   setup_timer_2(T2_DIV_BY_4,79,16);    //setup up timer2 to interrupt every 1ms if using 20Mhz clock
   enable_interrupts(INT_TIMER2);   //enable timer2 interrupt (if want to count ms)
   enable_interrupts(GLOBAL);       //enable all interrupts
   
   can_init();
   set_tris_b((*0xF93 & 0xFB) | 0x08);  //b3 is out, b2 is in (default)

   while(1) {
        getData();
        if(ms>1000) {
            ms = 0;
            //getTemp();

            //printData();
        }
   }
}
void getData(void) {
    //Use local receive structure for CAN polling receive
    struct rx_stat rxstat;
    int32 rx_id;
    unsigned int8 in_data[8]; //Data fields can have up to 8 bytes, byte range is from 0 to 255
    int rx_len;
    
    // If there is no CAN message waiting in the buffer, do nothing
    if (!can_kbhit())
        return;
    
    // If data is waiting in buffer...
    if(can_getd(rx_id, in_data, rx_len, rxstat)) {
        switch(rx_id) {
        //Motor Controller Packets
        case STATUS_ID:
            memcpy(statusData, in_data, 8);           
            break;
        case BUS_ID:
            memcpy(busData, in_data, 8);
            break;
        case VELOCITY_ID:
            memcpy(motorData, in_data, 4);
            break;
        case IPM_MOTOR_TEMP_ID:
            memcpy(motorIPMTempData, in_data, 8);
            break;
        case BOARD_TEMP_ID:
            memcpy(boardTempData, in_data, 4);
            break;
        case ODOMETER_ID:
            memcpy(odometerData, in_data, 4);
            break;
        
        //BPS Packets
        case BPS_VOLT1:
            memcpy(BPSVolt, in_data, 8);
            break;
        case BPS_VOLT2:
            memcpy(&BPSVolt[8], in_data, 6);
            memcpy(BPSCurr, &in_data[6], 2);
            break;
        case BPS_TEMP1:
            memcpy(BPSTemp, in_data, 8);
            break;
        case BPS_TEMP2:
            memcpy(&BPSTemp[8], in_data, 6);
            break;
        case BPS_ERROR:
            memcpy(BPSError, in_data, 6);
            break;
        }               
    } else {
        //printf("FAIL on can_getd\r\n");
    }
}

int8 getTemp(void) {
    int8 temp;
    
    i2c_start();
    i2c_write(72<<1);
    i2c_write(0);
    i2c_start();
    i2c_write((72<<1) + 1);
    temp = i2c_read(0);
    i2c_stop();
    return(temp);
}

/**void printData(void) {
    output_toggle(PIN_C0);
    
    printf("VehicleSpeed: %4.1f\r\n", vehicleSpeed);
    printf("BusVolt: %4.1f BusCurr: %4.1f\r\n", busVoltage, busCurrent);
    printf("MotorTemp: %4.1f BoardTemp: %4.1f IPMTemp: %4.1f\r\n\n", motorTemp, boardTemp, IPMTemp);
}8?

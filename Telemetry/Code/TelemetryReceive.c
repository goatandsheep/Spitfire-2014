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
//#FUSES CANC                   //Enable to move CAN pins to C6(TX) and C7(RX) 

#use delay(clock = 20000000)
#use rs232(baud = 115200, parity = N, UART1, bits = 8, ERRORS)
#define LED PIN_B5
#define MOTOR_CONT_ID 0x400
#define STATUS_ID         MOTOR_CONT_ID + 1
#define BUS_ID            MOTOR_CONT_ID + 2
#define VELOCITY_ID       MOTOR_CONT_ID + 3
#define MOTOR_VOLT_ID     MOTOR_CONT_ID + 5
#define MOTOR_CURR_ID     MOTOR_CONT_ID + 6
#define IPM_MOTOR_TEMP_ID MOTOR_CONT_ID + 11
#define BOARD_TEMP_ID     MOTOR_CONT_ID + 12

#include <can-18F4580_mscp.c>  // Modified CAN library includes default FIFO mode, timing settings match MPPT, 11-bit instead of 24-bit addressing

unsigned int16 ms = 0;
float motorSpeed, vehicleSpeed;
float motorVoltage, motorCurrent;
float busVoltage, busCurrent;
float motorTemp, boardTemp, IPMTemp;

void getData(void);
void printData(void);

//unused code at the moment
unsigned int8 out_data[8];
int32 tx_id = 0x900;
int1 tx_rtr = 1;
int1 tx_ext = 0;
int tx_len = 0;
int tx_pri = 3;

// Interrupt service routines
#int_timer2
void isr_timer2(void) {
    ms++;
}

void main(void) {
   
   //enable_interrupts(INT_CANRX0);   //enable CAN FIFO receive interrupt
   //enable_interrupts(INT_CANRX1);   //enable CAN FIFO receive interrupt
   setup_timer_2(T2_DIV_BY_4,79,16);    //setup up timer2 to interrupt every 1ms if using 20Mhz clock
   enable_interrupts(INT_TIMER2);   //enable timer2 interrupt (if want to count ms)
   enable_interrupts(GLOBAL);       //enable all interrupts
   
   can_init();
   set_tris_b((*0xF93 & 0xFB) | 0x08);  //b3 is out, b2 is in (default)
   //set_tris_c((*0xF94 & 0xBF) | 0x80);  //c6 is out, c7 is in (if using #FUSES CANC)
   //can_set_mode(CAN_OP_LOOPBACK);       //Only use for loopback testing

   while(1) {
        getData();
        if(ms>1000) {
            printData();
        }
   }
}  
void getData(void) {
        
        //Use local receive structure for CAN polling receive
        struct rx_stat rxstat;
        int32 rx_id;
        unsigned int8 in_data[8]; //Data fields can have up to 8 bytes, byte range is from 0 to 255
        int rx_len;
        
        // This is the polling receive routine
        if (can_kbhit()) {
            // If data is waiting in buffer...
            if(can_getd(rx_id, in_data, rx_len, rxstat)) {                   
                // get data from buffer, and place it into the fields: rx_id, in_data, rx_len... etc
                printf("RECIEVED: BUFF=%U ID=%3LX LEN=%U OVF=%U ", rxstat.buffer, rx_id, rx_len, rxstat.err_ovfl);
                printf("FILT=%U RTR=%U EXT=%U INV=%U\r\n", rxstat.filthit, rxstat.rtr, rxstat.ext, rxstat.inv);
                    
                if(rx_id == BUS_ID) {
                    busVoltage = (int32)in_data[4]<<12;
                    busVoltage += (int32)in_data[5]<<8;
                    busVoltage += (int32)in_data[6]<<4;
                    busVoltage += (int32)in_data[7];
                    busCurrent = (int32)in_data[0]<<12;
                    busCurrent += (int32)in_data[1]<<8;
                    busCurrent += (int32)in_data[2]<<4;
                    busCurrent += (int32)in_data[3];
                } else if(rx_id == VELOCITY_ID) {
                    motorSpeed = (int32)in_data[4]<<12;
                    motorSpeed += (int32)in_data[5]<<8;
                    motorSpeed += (int32)in_data[6]<<4;
                    motorSpeed += (int32)in_data[7];
                    vehicleSpeed = (int32)in_data[0]<<12;
                    vehicleSpeed += (int32)in_data[1]<<8;
                    vehicleSpeed += (int32)in_data[2]<<4;
                    vehicleSpeed += (int32)in_data[3];
                } else if(rx_id == MOTOR_VOLT_ID) {
                    motorVoltage = (int32)in_data[0]<<12;
                    motorVoltage += (int32)in_data[1]<<8;
                    motorVoltage += (int32)in_data[2]<<4;
                    motorVoltage += (int32)in_data[3];
                } else if(rx_id == MOTOR_CURR_ID) {
                    motorCurrent = (int32)in_data[0]<<12;
                    motorCurrent += (int32)in_data[1]<<8;
                    motorCurrent += (int32)in_data[2]<<4;
                    motorCurrent += (int32)in_data[3];
                } else if(rx_id == IPM_MOTOR_TEMP_ID) {
                    motorTemp = (int32)in_data[4]<<12;
                    motorTemp += (int32)in_data[5]<<8;
                    motorTemp += (int32)in_data[6]<<4;
                    motorTemp += (int32)in_data[7];
                    IPMTemp = (int32)in_data[0]<<12;
                    IPMTemp += (int32)in_data[1]<<8;
                    IPMTemp += (int32)in_data[2]<<4;
                    IPMTemp += (int32)in_data[3];
                } else if(rx_id == BOARD_TEMP_ID) {
                    boardTemp = (int32)in_data[4]<<12;
                    boardTemp += (int32)in_data[5]<<8;
                    boardTemp += (int32)in_data[6]<<4;
                    boardTemp += (int32)in_data[7];
                }
            } else {
                printf("FAIL on can_getd");
            }      
            printf("\r\n");
        }
}

void printData(void) {
    printf("MotorSpeed: %3.1f VehicleSpeed: %3.1f\r\n", motorSpeed, vehicleSpeed);
    printf("MotorVolt: %3.1f MotorCurr: %3.1f\r\n", motorVoltage, motorCurrent);
    printf("BusVolt: %3.1f BusCurr: %3.1f\r\n", busVoltage, busCurrent);
    printf("MotorTemp: %3.1f BoardTemp: %3.1f IPMTemp: %3.1f\r\n", motorTemp, boardTemp, IPMTemp);
}

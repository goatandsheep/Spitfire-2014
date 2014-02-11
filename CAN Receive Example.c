/*
 * This is example code for a RECEIVING node
 *
 *
 * Relevant Interrupts:
 * 
 * #int_canirx  
 * Invalid packet is received.
 * 
 * #int_canwake
 * PIC woke up by activity on the CANbus.
 * 
 * #int_canerr
 * There is an error in the CAN module.
 * 
 * #int_cantx0
 * Transmission from buffer 0 has completed.
 * 
 * #int_cantx1
 * Transmission from buffer 1 has completed.
 * 
 * #int_cantx2
 * Transmission from buffer 2 has completed.
 * 
 * #int_canrx0
 * Message is received in buffer 0.
 * 
 * #int_canrx1
 * Message is received in buffer 1. 
 *
 * Relevant Include Files:  
 *
 * can-mcp2510.c - Drivers for the MCP2510 and MCP2515 interface chips.
 * 
 * can-18xxx8.c -  Drivers for the built in CAN module.
 * 
 * can-18F4580.c - Drivers for the built in ECAN module. (Extended CAN)
 */

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
//#FUSES CANC                     //Enable to move CAN pins to C6(TX) and C7(RX) 

#use delay(clock = 20000000)
#use rs232(baud = 115200, parity = N, UART1, bits = 8, ERRORS)
#define LED PIN_C0
#define RTS PIN_C5

#include <can-18F4580_mscp.c>  // Modified CAN library includes default FIFO mode, timing settings match MPPT, 11-bit instead of 24-bit addressing


void main() {
   //Use local receive structure for CAN polling receive
   struct rx_stat rxstat;
   int32 rx_id;
   // Data fields can have up to 8 bytes, byte range is from 0 to 255
   int8 in_data[8];
   int rx_len;
   
   int i;
   

   //setup_timer_2(T2_DIV_BY_4,79,16);    //setup up timer2 to interrupt every 1ms if using 20Mhz clock

   can_init();
   set_tris_b((*0xF93 & 0xFB) | 0x08);  //b3 is out, b2 is in (default)
   //set_tris_c((*0xF94 & 0xBF) | 0x80);  //c6 is out, c7 is in (if using #FUSES CANC)
   //can_set_mode(CAN_OP_LOOPBACK);       //Only use for loopback testing
   
   //enable_interrupts(INT_CANRX1);   //enable CAN FIFO receive interrupt
   enable_interrupts(INT_TIMER2);   //enable timer2 interrupt (if want to count ms)
   enable_interrupts(GLOBAL);       //enable all interrupts

    while(1) {
        output_high(RTS);
        
        delay_ms(800);
      
        // This is the polling receive routine
        if (can_kbhit()) {
            //if data is waiting in buffer...
            if(can_getd(rx_id, in_data, rx_len, rxstat)) {
                //...then get data from buffer, and place it into the fields: rx_id, in_data, rx_len... etc
                printf("RECIEVED: BUFF=%U ID=%3LX LEN=%U OVF=%U ", rxstat.buffer, rx_id, rx_len, rxstat.err_ovfl);
                printf("FILT=%U RTR=%U EXT=%U INV=%U\r\n", rxstat.filthit, rxstat.rtr, rxstat.ext, rxstat.inv);
                printf("\tDATA = ");
                for (i=0;i<rx_len;i++)
                    printf("%02X ", in_data[i]);
            } else {
                printf("FAIL on can_getd\r\n");
            }
            
            printf("\r\n");
        }
    }
}


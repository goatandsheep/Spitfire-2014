/**McMaster University Solar Car Club
  *Fireball II January 20 2014
  *Battery Protection Team
  *-------------------------
  */

#include <18F26K80.h>
#device adc = 16

#FUSES NOWDT 
#FUSES HSH
#FUSES NOPLLEN

#FUSES SOSC_DIG 
#FUSES NOXINST 
#FUSES BROWNOUT               
#FUSES PUT
#FUSES NOIESO
#FUSES NOFCMEN
#FUSES NOPROTECT
#FUSES CANC

#define LED PIN_A5
#define BATT_RELAY PIN_A0
#define MPPT_RELAY PIN_C0

#define RS232_XMIT_PIN PIN_C6
#define RS232_RCV_PIN PIN_C7
#define I2C_SDA_PIN PIN_C4
#define I2C_SCL_PIN PIN_C3
#define I2C_WRITE_BIT 0
#define I2C_READ_BIT 1
#define ADC_CHANNEL 1

#use delay(clock = 20000000)
#use rs232(baud = 115200, parity = N, UART1, bits = 8)
#use i2c(master, sda = I2C_SDA_PIN, scl = I2C_SCL_PIN)

#include <can-18F4580_mscp.c>
#include <flex_lcd_PROT4-2.c>

typedef unsigned int8 uint8;
typedef unsigned int16 uint16;

uint16 rawVoltageRetrieve(uint8 sensorNum);
uint16 rawTemperatureRetrieve(uint8 sensorNum);
uint16 rawCurrentRetrieve(void);

const uint16 OVER_VOLTAGE = 0;
const uint16 UNDER_VOLTAGE = 0;
const uint16 OVER_TEMPERATURE = 0;
const uint16 OVER_CURRENT = 0;

const uint16 updateInterval = 1000;
const uint8 totalSensors = 14;
const uint8 sensorIDs[totalSensors] = {0x05,0,0,0,0,0,0,0,0,0,0,0,0,0};
uint16 rawVoltageData[totalSensors] = {1,2,3,4,5,6,7,100,120,50,120,105,101,100};
uint16 rawTemperatureData[totalSensors] = {6,10,12,53,24,45,66,37,48,39,410,311,312,213};
uint16 rawCurrentData = 0;
uint16 minVoltage = 0;
uint16 maxVoltage = 0;
uint16 maxTemperature = 0;

uint16 ms = 0;
int8 packetNum = 1;

const int32 tx_id = 0x002;
//uint8 out_data[8] = {1,2,3,4,100,200,300,400};
const int tx_len = 8;
const int tx_pri = 3;
const int1 tx_rtr = 0;
const int1 tx_ext = 0;

#int_timer2
void isr_timer2(void) {
   ms++;
}

//send data to LCD
void LCD_Send(void) {
    //printf(lcd_putc, "Hello World!");
}

void setPacketData(int8 packet, uint8* data) {
    int i;
    uint8 truncValue;
    if(packet == 1) {
            data[0] = 0;
            // for loop works when taking value from uint8 array to data(uint8 array), but not from uint16 array to data
            for(i = 1; i < 8; i++) {
               // truncValue = (uint8)rawVoltageData[i-1];
                data[i] = rawVoltageData[i-1];
            }
            /**for(i = 5; i < 8; i++)
                data[i] = (uint8)rawVoltageData[i-1];
                
            /*data[1] = rawVoltageData[1-1]>>2;
            data[2] = rawVoltageData[2-1]>>2;
            data[3] = rawVoltageData[3-1]>>2;
            data[4] = rawVoltageData[4-1]>>2;
            data[5] = rawVoltageData[5-1]>>2;
            data[6] = rawVoltageData[6-1]>>2;
            data[7] = rawVoltageData[7-1]>>2;*/
    } else if (packet == 2) {
            data[0] = rawCurrentData>>8;
            data[1] = rawVoltageData[7]>>2;
            data[2] = rawVoltageData[8]>>2;
            data[3] = rawVoltageData[9]>>2;
            data[4] = rawVoltageData[10]>>2;
            data[5] = rawVoltageData[11]>>2;
            data[6] = rawVoltageData[12]>>2;
            data[7] = rawVoltageData[13]>>2;
    } else if (packet == 3) { 
            data[0] = rawCurrentData;
            data[1] = rawTemperatureData[0];
            data[2] = rawTemperatureData[1]>>2;
            data[3] = rawTemperatureData[2]>>2;
            data[4] = rawTemperatureData[3]>>2;
            data[5] = rawTemperatureData[4]>>2;
            data[6] = rawTemperatureData[5]>>2;
            data[7] = rawTemperatureData[6]>>2;
    } else if (packet == 4) {
            data[0] = rawTemperatureData[7]>>2;
            data[1] = rawTemperatureData[8]>>2;
            data[2] = rawTemperatureData[9]>>2;
            data[3] = rawTemperatureData[10]>>2;
            data[4] = rawTemperatureData[11]>>2;
            data[5] = rawTemperatureData[12]>>2;
            data[6] = rawTemperatureData[13]>>2;
            data[7] = 0;
    }
}

//send data to CANBus
int CANBus_Send(void) {
    int8 response;
    uint8 out_data[8];

    setPacketData(packetNum, &out_data[0]);
    response = can_putd(tx_id,out_data,tx_len,tx_pri,tx_ext,tx_rtr);
    if(response != 0xFF) {
        output_toggle(LED);
    }
    /**if(packetNum == 4) {
        packetNum = 1;
    } else {
        packetNum++;
    }*/
    return(response);
}

void main(void) {
   setup_comparator(NC_NC_NC_NC);
   setup_adc(ADC_CLOCK_DIV_32);
   setup_adc_ports(ALL_ANALOG);
   set_adc_channel(ADC_CHANNEL);
   
   setup_timer_2(T2_DIV_BY_4,79,16);
   enable_interrupts(INT_TIMER2);  
   enable_interrupts(GLOBAL); 
    
   can_init();
   set_tris_c((*0xF94 & 0xBF) | 0x80);

   while(true) {
        
        /**retrieve raw data and store in rawSensorData and rawCurrentData
        for(i = 0; i < totalSensors; i++) {
            rawVoltageData[i] = rawVoltageRetrieve(sensorIDs[i]);
            rawTemperatureData[i] = rawTemperatureRetrieve(sensorIDs[i]);
        }
        rawCurrentData = rawCurrentRetrieve();*/
        
        if(ms > updateInterval) {
            ms = 0; //set interrupt timer to 0
            
            LCD_Send();
            CANBus_Send();
            //output_toggle(BATT_RELAY);            

       }
   }
}

uint16 combineInts(uint8 high, uint8 low) {
    uint16 r = high;
    r = r << 3;
    low = low >> 5;
    r = r + low;
    
    return (r);
}

uint16 rawVoltageRetrieve(uint8 sensorNum) {
    uint8 temp0, temp1;
    sensorNum<<=1;
    
    i2c_start();
        i2c_write(sensorNum + I2C_WRITE_BIT);
        i2c_write(0x0C);
    i2c_start();
        i2c_write(sensorNum + I2C_READ_BIT);
        temp0 = i2c_read(0);
    i2c_stop();
    
    i2c_start();
        i2c_write(sensorNum + I2C_WRITE_BIT);
        i2c_write(0x0D);
    i2c_start();
        i2c_write(sensorNum + I2C_READ_BIT);
        temp1 = i2c_read(0);
    i2c_stop();
    
    return(combineInts(temp0,temp1));
}

uint16 rawTemperatureRetrieve(uint8 sensorNum) {
    uint8 temp0, temp1;
    sensorNum<<=1;
    
    i2c_start();
        i2c_write(sensorNum + I2C_WRITE_BIT);
        i2c_write(0x18);
    i2c_start();
        i2c_write(sensorNum + I2C_READ_BIT);
        temp0 = i2c_read(0);
    i2c_stop();
    
    i2c_start();
        i2c_write(sensorNum + I2C_WRITE_BIT);
        i2c_write(0x19);
    i2c_start();
        i2c_write(sensorNum + I2C_READ_BIT);
        temp1 = i2c_read(0);
    i2c_stop();
    
    return(combineInts(temp0,temp1));
}

uint16 rawCurrentRetrieve(void) {
    uint16 rawCurrent;
    rawCurrent = read_adc();
    //sensor result conversion pending
    
    return(rawCurrent);
}

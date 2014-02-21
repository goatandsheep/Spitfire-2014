/* McMaster University Solar Car Club
 * Fireball II January 20 2014
 * Battery Protection Team
 * -------------------------
 */

#include <18F26K80.h>
#device adc = 16

#FUSES NOWDT            // No Watch Dog Timer
#FUSES HSH              // High speed Osc, high power 16MHz-25MHz
#FUSES NOPLLEN          // 4X HW PLL disabled, 4x PLL enabled in software

#FUSES SOSC_DIG         // Digital mode, I/O port functionality of RC0 and RC1
#FUSES NOXINST          // Extended set extension and Indexed Addressing mode disabled(Legacy mode)
#FUSES BROWNOUT         // Reset when brownout detected
#FUSES PUT              // Power Up Timer
#FUSES NOIESO           // Internal External Switch Over mode disabled
#FUSES NOFCMEN          // Fail-safe clock monitor disabled
#FUSES NOPROTECT        // Code not protected from reading
#FUSES CANC             // CANTX and CANRX pins are located on RC6 and RC7

#include <can-18F4580_mscp.c>
//#include <flex_lcd_PROT4-2.c>

// Preprocessor Macros
#define LED PIN_A5
#define BATT_RELAY PIN_A0
#define MPPT_RELAY PIN_C0
#define SENSOR_RELAY PIN_C5

#define I2C_SDA_PIN PIN_C4
#define I2C_SCL_PIN PIN_C3
#define I2C_WRITE_BIT 0
#define I2C_READ_BIT 1
#define ADC_CHANNEL 1

// Threshold values for voltage, temperature and current
#define VOLT_MAX_WARN   4.0
#define VOLT_MAX_ERROR  4.2
#define VOLT_MIN_WARN   3.0
#define VOLT_MIN_ERROR  2.8   // paul told me to write a comment
#define TEMP_CHARGE_MAX_WARN   40
#define TEMP_CHARGE_MAX_ERROR  45
#define TEMP_DISCHARGE_MAX_WARN   55
#define TEMP_DISCHARGE_MAX_ERROR  60
#define CURRENT_CHARGE_MAX_WARN   0 //to be announced
#define CURRENT_CHARGE_MAX_ERROR  0
#define CURRENT_DISCHARGE_MAX_WARN   0
#define CURRENT_DISCHARGE_MAX_ERROR  0
#define VOLT_REGISTER 0x0C
#define TEMP_REGISTER 0x18

// Error codes
#define NO_ERROR 0x00
#define ERROR_HIGH_VOLT 0x01

// Typedefs
typedef unsigned int8 uint8;
typedef unsigned int16 uint16;

// Function prototypes
void LCD_Send(void);
int CANBus_SendData(void);
uint8 checkSensorValues(uint8 n);
int CANBus_SendMessage(uint8 errCode, uint8 sensor);
void setPacketData(int8 packet, uint8* data);
uint16 getRawSensorVal(uint8 sensorNum, uint8 sensorRegister);
uint16 rawCurrentRetrieve(void);
uint16 combineBytes(uint8 high, uint8 low);

#use delay(clock = 20000000)
#use rs232(baud = 115200, parity = N, UART1, bits = 8)
#use i2c(master, sda = I2C_SDA_PIN, scl = I2C_SCL_PIN)

const uint16 updateInterval = 250; // ms
const uint8 maxErrorReadings = 4;
const uint8 totalSensors = 14;
const uint8 sensorIDs[totalSensors] = {0x05,0,0,0,0,0,0,0,0,0,0,0,0,0};
uint16 rawVoltData[totalSensors] = {4,8,12,16,20,24,28,32,36,40,44,48,52,56};
uint16 rawTempData[totalSensors] = {64,68,72,76,80,84,88,92,96,100,104,108,112,116};
uint16 rawCurrentData = 0xABCD;
uint16 minVoltage = 0;
uint16 maxVoltage = 0;
uint16 maxTemperature = 0;

uint16 ms = 0;
int8 packetNum = 0;

const int32 tx_id = 0x002;  // bps id?
const int tx_len = 8;   // either not needed or not constant, int literal is probably good enough
const int tx_pri = 3;
const int1 tx_rtr = 0;
const int1 tx_ext = 0;

// Interrupt service routines
#int_timer2
void isr_timer2(void) {
   ms++;
}

// Use length of can frame to tell wether or not its an error message
// 8 bytes is normal data flow, less is error message
// Over/Under Volt/Temp - Send error code, sensor, and reading. (3 Bytes)
// Over/Under current - Send error code and reading. (2 Bytes)

void main(void) {
    uint16 i;
    uint8 warnFlag = 0;
    uint8 voltErrorCount[totalSensors];
    uint8 tempErrorCount[totalSensors];
    uint8 currentErrorCount;

    output_high(LED);
    setup_comparator(NC_NC_NC_NC);
    setup_adc(ADC_CLOCK_DIV_32);
    setup_adc_ports(ALL_ANALOG);
    set_adc_channel(ADC_CHANNEL);
   
    setup_timer_2(T2_DIV_BY_4,79,16);
    enable_interrupts(INT_TIMER2);  
    enable_interrupts(GLOBAL); 
    
  
            // Retrieve raw data and store in rawSensorData and rawCurrentData
            rawVoltData[0] = getRawSensorVal(0x05, VOLT_REGISTER);
            
            for(i = 0; i < totalSensors; i++) {
                rawVoltData[i] = getRawSensorVal(sensorIDs[i], VOLT_REGISTER);
                rawTempData[i] = getRawSensorVal(sensorIDs[i], TEMP_REGISTER);
                
                uint8 errCode = checkSensorValues(i);
                
                if (errCode != NO_ERROR) {
                    // do warnings and errors
                    CANBus_SendMessage(errCode, i);
                      can_init();
    for(i=0;i<totalSensors;i++) {
        voltErrorCount[i]=0;
        tempErrorCount[i]=0;
    }
    currentErrorCount=0;
    
    output_high(SENSOR_RELAY); //set sensor relay to high
    while(true) {
    set_tris_c((*0xF94 & 0xBF) | 0x80); // Set C7 to input, C6 to output
    
    // Initialize error counts to 0
        // Send data to the CANBus only every second
        if(ms > updateInterval) {
            ms = 0;
            
                    if (voltErrorCount[i] > maxErrorCount) {
                        // shutdown
                    }
                
                    if (tempErrorCount[i] > maxErrorCount) {
                        // shutdown
                    }
                }
            }
            rawCurrentData = rawCurrentRetrieve();
  
            LCD_Send();
            CANBus_SendData();
       }
   }
}

// Send data to LCD
void LCD_Send(void) {
    //printf(lcd_putc, "Hello World!");
}

int CANBus_SendData(void) {
    int8 response;
    // does this even need to be initd to 0s?
    uint8 out_data[8] = {0,0,0,0,0,0,0,0};

    setPacketData(packetNum, out_data);
    response = can_putd(tx_id,out_data,8,tx_pri,tx_ext,tx_rtr);
     
    // Check if the data was written successfully
    if(response != 0xFF) {
        output_toggle(LED);
        
        //packetNum = (packetNum + 1) % 4;
    }
    
    return(response);
}

uint8 checkSensorValues(uint8 n) {
    // First check voltage data
    const uint16 volt = rawVoltData[n];
    const uint16 temp = rawTempData[n];
    
    if (volt > VOLT_MAX_WARN) {
        warnFlag = 1;
    }
    
    if (voltErrorCount[n] == 0 && volt < VOLT_MAX_WARN) {
        warnFlag = 0;
    }

    return NO_ERROR;
}

int CANBus_SendMessage(uint8 errCode, uint8 sensor) {
    if (errCode == NO_ERROR)
        return 0;

    int8 response;
    uint8 out_data[3];
    
    out_data[0] = errCode;
    out_data[1] = sensor;
    // out_data[2] = sensor reading
    
    response = can_putd(tx_id,out_data,3,tx_pri,tx_ext,tx_rtr);
    
    
    return (response);
}

void setPacketData(int8 packet, uint8* data) {
    uint16 i;
    if(packet == 0) {
        data[0] = 0;
        
        for(i = 1; i < 8; i++)
            data[i] = (uint8)(rawVoltData[i-1]>>2);
    } else if (packet == 1) {
        data[0] = (uint8)(rawCurrentData>>8);       
        for(i = 1; i < 8; i++)
            data[i] = (uint8)(rawVoltData[i+6]>>2);
    } else if (packet == 2) {
        data[0] = (uint8)rawCurrentData;       
        for(i = 1; i < 8; i++)
            data[i] = (uint8)(rawTempData[i-1]>>2);
    } else if (packet == 3) {
        for(i = 0; i < 7; i++)
            data[i] = (uint8)(rawTempData[i+7]>>2);            
        data[7] = 0;
    }
}

// See DS2764 datasheet - Voltage/Temperature Measurement
uint16 getRawSensorVal(uint8 sensorNum, uint8 sensorRegister) {
    uint8 loBits, hiBits;
    sensorNum<<=1;
    
    // Read most significant byte, contains bits 3-9 
    i2c_start();
    i2c_write(sensorNum + I2C_WRITE_BIT);
    i2c_write(sensorRegister);
    i2c_start();
    i2c_write(sensorNum + I2C_READ_BIT);
    hiBits = i2c_read(0);
    i2c_stop();
    
    // Read least significant byte, contains bits 0-2
    i2c_start();
    i2c_write(sensorNum + I2C_WRITE_BIT);
    i2c_write(sensorRegister + 1);
    i2c_start();
    i2c_write(sensorNum + I2C_READ_BIT);
    loBits = i2c_read(0);
    i2c_stop();
    
    return(combineBytes(hiBits,loBits));
}

uint16 rawCurrentRetrieve(void) {
    uint16 rawCurrent;
    rawCurrent = read_adc();
    //sensor result conversion pending
    
    return(rawCurrent);
}

uint16 combineBytes(uint8 high, uint8 low) {
    // Quick bit masking sanity check to make sure we're never returning ints
    // greater than 2^10
    uint16 top = ((uint16)high << 3) & 0x03F8;
    uint16 bot = ((uint16)low >> 5) & 0x0007;
    
    return (top + bot);
}

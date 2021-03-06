/* McMaster University Solar Car Club
 * Fireball II January 20 2014
 * Battery Protection Team
 *
 * Paul Correia
 * Lun Li
 * -------------------------
 * Version History:
 *
 * 03/24/2014 - Uncalibrated Beta
 */

#include <18F26K80.h>   //Library for PIC used (18F26K80)
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

// Preprocessor Macros
// uC pins
#define LED             PIN_A5
#define BATT_RELAY      PIN_A0
#define MPPT_RELAY      PIN_C0
#define SENSOR_RELAY    PIN_C5
#define I2C_SDA_PIN     PIN_C4
#define I2C_SCL_PIN     PIN_C3

#define I2C_WRITE_BIT 0
#define I2C_READ_BIT 1
#define ADC_CHANNEL 1

// Voltage and temperature sensor registers (check sensor DATASHEET for more info
#define VOLT_REGISTER   0x0C
#define TEMP_REGISTER   0x18

// Threshold values for voltage, temperature and current
// To convert voltage values, divide by 5.0V and multiply by 1023
// To convert temperature values, divide by 127 degC and multiply by 1023
#define VOLT_MAX_WARN   818     // 4.0V
#define VOLT_MAX_ERROR  860     // 4.2V
#define VOLT_MIN_WARN   717     // 3.0V
#define VOLT_MIN_ERROR  573     // 2.8V
#define TEMP_CHARGE_MAX_WARN        225 // degC
#define TEMP_CHARGE_MAX_ERROR       240 // degC
#define TEMP_DISCHARGE_MAX_WARN     225 // 28degC
#define TEMP_DISCHARGE_MAX_ERROR    240 // 30degC
#define CURRENT_CHARGE_MAX_WARN         0 //to be announced
#define CURRENT_CHARGE_MAX_ERROR        0
#define CURRENT_DISCHARGE_MAX_WARN      0
#define CURRENT_DISCHARGE_MAX_ERROR     0

// Error and warning codes
#define NONE            0x00
#define VOLT_HIGH       0x01
#define VOLT_LOW        0x02
#define TEMP_HIGH       0x04
#define TEMP_LOW        0x08
#define CURRENT_HIGH    0x10
#define CURRENT_LOW     0x20
#define WARN_BIT        0x40
#define ERROR_BIT       0x80

// Code checking
#define IS_VOLT_CODE(x)     ((x) & (VOLT_HIGH | VOLT_LOW))
#define IS_TEMP_CODE(x)     ((x) & (TEMP_HIGH | TEMP_LOW))
#define IS_CURRENT_CODE(x)  ((x) & (CURRENT_HIGH | CURRENT_LOW))
#define IS_WARNING(x)       ((x) & WARN_BIT)
#define IS_ERROR(x)         ((x) & ERROR_BIT)

// Typedefs
typedef unsigned int8 uint8;
typedef unsigned int16 uint16;

typedef struct SensorData {
    uint8 ID;
    uint16 voltData, tempData;
    int1 overVoltFlag, underVoltFlag, overTempFlag;
    uint8 overVoltCount, underVoltCount;
    uint8 overTempCount;
};

typedef struct HallEffectData {
    uint16 data;
    uint8 overCount;
    int1 overFlag;
};

#use delay(clock = 20000000)
#use i2c(master, sda = I2C_SDA_PIN, scl = I2C_SCL_PIN)
#include <../../Shared/Code/can-18F4580_mscp.c>  //CAN library
#include <../../Shared/Code/flex_lcd_library.c>  //LCD library

const uint16 updateInterval = 1000; // ms
const uint8 maxErrorCount = 4;
const uint8 numSensors = 14;
struct SensorData sensor[numSensors];
struct HallEffectData hallSensor;
uint16 rawCurrentData = 0xABCD;

// Keep track of sensor with greatest/least voltage and temperature, and their
// corresponding sensor readings, so we can print them to the LCD
uint16 minVolt, maxVolt, maxTemp;
uint8 minVoltNum, maxVoltNum, maxTempNum;

uint16 ms = 0;
int8 packetNum = 0;

const int32 tx_id = 0x800;  // To be changed later
const int tx_pri = 3;
const int1 tx_rtr = 0;
const int1 tx_ext = 0;

// Function prototypes
void setup(void);
void initSensors(void);
uint8 updateSensor(uint8 n);

void LCD_Send(void);
int CANBus_SendData(void);
int setPacketData(int8 packet, uint8* data);

uint8 getErrCode(uint16 voltReading, uint16 tempReading);
uint8 getHallErrorCode(uint16 hallReading);
int1 hallDischarge(void);

uint16 getRawSensorVal(uint8 sensorNum, uint8 sensorRegister);
uint16 getRawCurrentVal(void);
uint16 combineBytes(uint8 high, uint8 low);

// Interrupt service routines
#int_timer2
void isr_timer2(void) {
    ms++;
}

// Use length of can frame to tell wether or not its an error message
// 8 bytes is normal data flow, less is error message
// Over/Under Volt/Temp - Send error code, sensor, and reading. (3 Bytes)
// Over/Under current - Send error code and reading. (2 Bytes)

//setup
void setup(void) {
    output_high(LED);
    output_high(BATT_RELAY);
    output_high(MPPT_RELAY);
    
    setup_comparator(NC_NC_NC_NC);
    setup_adc(ADC_CLOCK_DIV_32);
    setup_adc_ports(ALL_ANALOG);
    set_adc_channel(ADC_CHANNEL);
    
    setup_timer_2(T2_DIV_BY_4,79,16);
    enable_interrupts(INT_TIMER2);
    enable_interrupts(GLOBAL);
    
    can_init();    //initialize CANBus
    lcd_init();    //initialize LCD
    initSensors();  //set sensor IDs and default values
    
    set_tris_c((*0xF94 & 0xBF) | 0x80); // Set C7 to input, C6 to output
    
    // Close relay on each sensor to physically enable it
    output_high(SENSOR_RELAY);
}

/*
 *  initSensors
 *
 *  Initializes the sensorIDs, sets the error counts and data values to 0 for
 *  every temperature/voltage sensor, as well as the hall effect sensor.
 */
void initSensors(void) {
    int i;

    // Initialize volt/temp sensors
    sensor[0].ID = 0x05;
    sensor[1].ID = 0x15;
    sensor[2].ID = 0x0;
    sensor[3].ID = 0x0;
    sensor[4].ID = 0x0;
    sensor[5].ID = 0x0;
    sensor[6].ID = 0x0;
    sensor[7].ID = 0x0;
    sensor[8].ID = 0x0;
    sensor[9].ID = 0x0;
    sensor[10].ID = 0x0;
    sensor[11].ID = 0x0;
    sensor[12].ID = 0x0;
    sensor[13].ID = 0x0;

    for (i=0; i<numSensors; i++) {
        sensor[i].tempData = 0;
        sensor[i].voltData = 0;
        sensor[i].overTempCount = 0;
        sensor[i].overVoltCount = 0;
        sensor[i].underVoltCount = 0;
    }
    
    // Initialize current sensor
    hallSensor.data = 0xABCD;
    hallSensor.overCount = 0;
}

void main(void) {
    
    setup();
    
    uint8 i;
    uint8 warnFlag = 0;
    uint8 errCode;
    
    output_low(LED);
    
    while(1) {
        if (ms <= updateInterval)
            continue;
        // else
        ms = 0;
        output_low(LED);
        
        // Reset min and max volt/temp
        minVolt=0xFFFF;
        maxVolt=0;
        maxTemp=0;
        
        for(i = 0; i < numSensors; i++) {
            errCode = updateSensor(i);
            
            if (sensor[i].voltData > maxVolt) {
                maxVolt = sensor[i].voltData;
                maxVoltNum = i;
            }
            if (sensor[i].voltData < minVolt) {
                minVolt = sensor[i].voltData;
                minVoltNum = i;
            }
            if (sensor[i].tempData > maxTemp) {
                maxTemp = sensor[i].tempData;
                maxTempNum = i;
            }
            
            // If the sensor is in the normal operating range, move on to the
            // next one
            if (errCode == NONE)
                continue;
            
            if (IS_ERROR(errCode)) {
                // Check if any of the counters have gone over the max count
                if (sensor[i].overVoltCount >= maxErrorCount ||
                    sensor[i].underVoltCount >= maxErrorCount ||
                    sensor[i].overTempCount >= maxErrorCount) {
                    // If ANY of the sensors have an error, turn off the relays
                    // They can never be turned back on until the program is
                    // reset.
                    output_low(BATT_RELAY);
                    output_low(MPPT_RELAY);  
                    output_high(LED); 
                }
            }
        }
        
        rawCurrentData = getRawCurrentVal();
                
        LCD_Send();
        CANBus_SendData();
    }
}

/*
 *  updateSensor
 *
 *  Reads the specified sensor's voltage and temperature values, compares these
 *  readings to their corresponding threshold values. If any readings are above
 *  their ERROR threshold, their error counter is incremented.
 *
 *  Arguments:
 *  n - Sensor number
 *  
 *  Return:
 *  Error and/or warning code
 */
uint8 updateSensor(uint8 n) {
    uint8 err = NONE;
    
    sensor[n].voltData = getRawSensorVal(sensor[n].ID, VOLT_REGISTER);
    sensor[n].tempData = getRawSensorVal(sensor[n].ID, TEMP_REGISTER);
    sensor[n].overVoltFlag = 0;
    sensor[n].underVoltFlag = 0;
    sensor[n].overTempFlag = 0;
    
    err = getErrCode(sensor[n].voltData, sensor[n].tempData);
    
    // The behaviour when an error bit isn't set may be changed to decrement the
    // counter instead of reseting it.
       
    if (err & VOLT_HIGH) {
        sensor[n].overVoltFlag = 1;
    
        if (IS_ERROR(err))
            sensor[n].overVoltCount++;
        else 
            sensor[n].overVoltCount--;
    }
    
    if (err & VOLT_LOW) {
        sensor[n].underVoltFlag = 1;
    
        if (IS_ERROR(err))
            sensor[n].underVoltCount++;
        else 
            sensor[n].underVoltCount--;
    }
    
    if (err & TEMP_HIGH) {
        sensor[n].overTempFlag = 1;
    
        if (IS_ERROR(err))
            sensor[n].overTempCount++;
        else 
            sensor[n].overTempCount--;
    }
    
    return err;
}

float rawToVolt(int16 rawVolt) {
    return (float)rawVolt * 0.0048876;
}
float rawToTemp(int16 rawTemp) {
    return (float)rawTemp/0.12414;
}
float rawToCurr(int16 rawCurr) {
    return ((float)rawCurr - 32685.0) * 0.0014701;
}

// Send data to LCD
void LCD_Send(void) {
    char buffer[32];
    
    sprintf(buffer, "V:%4.2f #%02d, v:%4.2f #%2d", 
                    rawToVolt(maxVolt), maxVoltNum, 
                    rawToVolt(minVolt), minVoltNum);
    lcd_gotoxy(0,0);
    printf(lcd_putc, buffer);
    sprintf(buffer, "T:%4.1f #%02d, curr=%5.2f", 
                    rawToTemp(maxTemp), maxTempNum, 
                    rawToCurr(hallSensor.data));
    lcd_gotoxy(0,1);
    printf(lcd_putc, buffer);
}

/*
 *  CANBus_SendData
 *
 *  Sends normal sensor data along the CAN bus. Data is set in setPacketData()
 *  and is sent in this function. Also increments the packet number
 *
 *  Return:
 *  response from can_putd, 0xFF if write failed, otherwise returns buffer that
 *  can_putd wrote to
 *
 */
int CANBus_SendData(void) {
    int8 response;
    int tx_len;
    
    // does this even need to be initd to 0s? yes
    uint8 out_data[8] = {0,0,0,0,0,0,0,0};
    
    tx_len = setPacketData(packetNum, out_data);
    response = can_putd(tx_id+packetNum,out_data,tx_len,tx_pri,tx_ext,tx_rtr);
    
    // Check if the data was written successfully
    if(response != 0xFF) {
        //output_toggle(LED);    
        packetNum = 2;
        //packetNum = (packetNum + 1) % 5;
    }
    
    return(response);
}

int setPacketData(int8 packetNum, uint8* data) {
    uint16 i;
    int packetLength = 8;
    
    switch(packetNum) { 
    case 0:
        for(i = 0; i < 8; i++)
            data[i] = (uint8)(sensor[i].voltData>>2);
        break;
    case 1:
        for(i = 0; i < 6; i++)
            data[i] = (uint8)(sensor[i+8].voltData>>2);
        data[7] = (uint8)(rawCurrentData>>8);
        data[8] = (uint8)rawCurrentData;
        break;
    case 2:
        for(i = 0; i < 8; i++)
            data[i] = (uint8)(sensor[i].tempData>>2);
        break;
    case 3:
        packetLength = 6;
        for(i = 0; i < 6; i++)
            data[i] = (uint8)(sensor[i+8].tempData>>2);
        break;
    case 4:
        packetLength = 6;        
        // Sensor 0 to 7
        for (i = 0; i < 8; i++) {
            data[0] |= sensor[i].overVoltFlag << (7 - i);
            data[2] |= sensor[i].underVoltFlag << (7 - i);
            data[4] |= sensor[i].overTempFlag << (7 - i);
        }        
        // Sensor 8 to 13
        for (i = 0; i < 6; i++) {
            data[1] |= sensor[i+8].overVoltFlag << (7 - i);
            data[3] |= sensor[i+8].underVoltFlag << (7 - i);
            data[5] |= sensor[i+8].overTempFlag << (7 - i);
        }        
        data[5] |= hallSensor.overFlag;
        break;
    }
    return(packetLength);
}

uint8 getErrCode(uint16 voltReading, uint16 tempReading) {
    uint8 ret = NONE;
    uint16 tempWarnThresh, tempErrThresh;
    
    if (voltReading >= VOLT_MAX_WARN) {
        ret |= VOLT_HIGH | WARN_BIT;
        
        if (voltReading >= VOLT_MAX_ERROR)
            ret |= ERROR_BIT;
    } else if (voltReading <= VOLT_MIN_WARN){
        ret |= VOLT_LOW | WARN_BIT;
        
        if (voltReading <= VOLT_MIN_ERROR)
            ret |= ERROR_BIT;
    }
        
    if (hallDischarge()) {
        tempWarnThresh = TEMP_DISCHARGE_MAX_WARN;
        tempErrThresh = TEMP_DISCHARGE_MAX_ERROR;
    } else {
        tempWarnThresh = TEMP_CHARGE_MAX_WARN;
        tempErrThresh = TEMP_CHARGE_MAX_ERROR;
    }
        
    if (tempReading >= tempWarnThresh) {
        ret |= TEMP_HIGH | WARN_BIT;
        
        if (tempReading >= tempErrThresh)
            ret |= ERROR_BIT;
    }
    
    return ret;
}

uint8 getHallErrorCode(uint16 hallReading) {

    uint8 ret = NONE;
    uint16 warnThresh, errThresh;
    
    // get appropriate threshold for charge/discharge
    if (hallDischarge()) {
        warnThresh = CURRENT_DISCHARGE_MAX_WARN;
        errThresh = CURRENT_DISCHARGE_MAX_ERROR;
    } else {
        warnThresh = CURRENT_CHARGE_MAX_WARN;
        errThresh = CURRENT_CHARGE_MAX_ERROR;
    }
    
    if (hallReading > warnThresh) {
        ret |= CURRENT_HIGH | WARN_BIT;
        
        if (hallReading > errThresh)
            ret |= ERROR_BIT;
    }
    
    return ret;
}

/*
 *  hallDischarge
 *
 *  Return:
 *  1 if the hall sensor measures positive current, 0 otherwise
 *
 */
int1 hallDischarge(void) {
    return (hallSensor.data > 0);
}

//Retrieves and returns uint16 raw voltage or temperature value from sensor
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

//Retrieves and returns uint16 raw current value from hall effect sensor
uint16 getRawCurrentVal(void) {
    uint16 rawCurrent;
    rawCurrent = read_adc();
    
    return(rawCurrent);
}

//Combines the two bytes (uint8) received from sensor into one (uint16)
uint16 combineBytes(uint8 high, uint8 low) {
    /* Quick bit masking sanity check to make sure we're never returning ints
       greater than 2^10 */
    uint16 top = ((uint16)high << 3) & 0x03F8;
    uint16 bot = ((uint16)low >> 5) & 0x0007;
    
    return (top + bot);
}

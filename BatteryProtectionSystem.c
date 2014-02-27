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

// Voltage and temperature sensor registers
#define VOLT_REGISTER   0x0C
#define TEMP_REGISTER   0x18

// Threshold values for voltage, temperature and current
#define VOLT_MAX_WARN   4.0     // These should be 16bit hex/dec values, not floats
#define VOLT_MAX_ERROR  4.2
#define VOLT_MIN_WARN   3.0
#define VOLT_MIN_ERROR  2.8
#define TEMP_CHARGE_MAX_WARN   40
#define TEMP_CHARGE_MAX_ERROR  45
#define TEMP_DISCHARGE_MAX_WARN     55
#define TEMP_DISCHARGE_MAX_ERROR    60
#define CURRENT_CHARGE_MAX_WARN     0 //to be announced
#define CURRENT_CHARGE_MAX_ERROR    0
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
    uint16 tempData, voltData;
    uint8 overTempCount;
    uint8 overVoltCount, underVoltCount;
};

typedef struct HallEffectData {
    uint16 data;
    uint8 overCount;
};

// Function prototypes (organize these)
void initSensors(void);
uint8 updateSensor(uint8 n);
void LCD_Send(void);
int CANBus_SendData(void);
int1 currentDischarge(void);
uint8 getErrCode(uint16 voltReading, uint16 tempReading);
uint8 getHallErrorCode(uint16 hallReading);
int CANBus_SendMessage(uint8 errCode, uint8 n);
void setPacketData(int8 packet, uint8* data);
uint16 getRawSensorVal(uint8 sensorNum, uint8 sensorRegister);
uint16 rawCurrentRetrieve(void);
uint16 combineBytes(uint8 high, uint8 low);

#use delay(clock = 20000000)
#use rs232(baud = 115200, parity = N, UART1, bits = 8)
#use i2c(master, sda = I2C_SDA_PIN, scl = I2C_SCL_PIN)

const uint16 updateInterval = 250; // ms
const uint8 maxErrorCount = 4;
const uint8 numSensors = 14;
struct SensorData sensor[numSensors];
struct HallEffectData hallSensor;
uint16 rawCurrentData = 0xABCD; // can current be negative?

uint16 ms = 0;
int8 packetNum = 0;

const int32 tx_id = 0x002;  // bps id?
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
    uint8 i;
    uint8 warnFlag = 0;
    uint8 errCode;
    
    output_high(LED);
    setup_comparator(NC_NC_NC_NC);
    setup_adc(ADC_CLOCK_DIV_32);
    setup_adc_ports(ALL_ANALOG);
    set_adc_channel(ADC_CHANNEL);
    
    setup_timer_2(T2_DIV_BY_4,79,16);
    enable_interrupts(INT_TIMER2);
    enable_interrupts(GLOBAL);
    
    can_init();
    initSensors();
    
    set_tris_c((*0xF94 & 0xBF) | 0x80); // Set C7 to input, C6 to output
    
    // Close relay on each sensor to physically enable it
    output_high(SENSOR_RELAY);
    
    while(1) {
        if (ms <= updateInterval)
            continue;
        // else
        ms = 0;
        
        for(i = 0; i < numSensors; i++) {
            errCode = updateSensor(i);
            
            // If the sensor is in the normal operating range, move on to the
            // next one
            if (errCode == NONE)
                continue;
            
            if (IS_WARNING(errCode)) {
                /* I don't think we've fully decided on the functionality here.
                 * Here are a few possibilities:
                 * - Send a message every time a new warning turns on and every
                 *   time the warning turns off.
                 * - Send a message every time a new warning turns on and send
                 *   a message when every warning finally turns off.
                 * - Send a warning when a warning turns on, don't send again
                 *   until every warning turns off, send a message when this
                 *   happens.
                 */
            }
            
            if (IS_ERROR(errCode)) {
                // Check if any of the counters have gone over the max count
                if (sensor[i].overVoltCount >= maxErrorCount ||
                    sensor[i].underVoltCount >= maxErrorCount ||
                    sensor[i].overTempCount >= maxErrorCount) {
                    
                }
            }
        }
        
        rawCurrentData = rawCurrentRetrieve();
                
        LCD_Send();
        CANBus_SendData();
    }
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
    sensor[1].ID = 0x0;
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
    uint8 ret = NONE;
    
    sensor[n].voltData = getRawSensorVal(sensor[n].ID, VOLT_REGISTER);
    sensor[n].tempData = getRawSensorVal(sensor[n].ID, TEMP_REGISTER);
    
    ret = getErrCode(sensor[n].voltData, sensor[n].tempData);
    
    // The behaviour when an error bit isn't set may be changed to decrement the
    // counter instead of reseting it.
    
    if (IS_ERROR(ret) && (ret & VOLT_HIGH))
        sensor[n].overVoltCount++;
    else
        sensor[n].overVoltCount = 0;
    
    if (IS_ERROR(ret) && (ret & VOLT_LOW))
        sensor[n].underVoltCount++;
    else
        sensor[n].underVoltCount = 0;
        
    if (IS_ERROR(ret) && (ret & TEMP_HIGH))
        sensor[n].overTempCount++;
    else
        sensor[n].overTempCount = 0;
    
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

// Send data to LCD
void LCD_Send(void) {
    //printf(lcd_putc, "Hello World!");
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
    // does this even need to be initd to 0s?
    uint8 out_data[8] = {0,0,0,0,0,0,0,0};
    
    setPacketData(packetNum, out_data);
    response = can_putd(tx_id,out_data,8,tx_pri,tx_ext,tx_rtr);
    
    // Check if the data was written successfully
    if(response != 0xFF) {
        output_toggle(LED);
        
        packetNum = (packetNum + 1) % 4;
    }
    
    return(response);
}

uint8 getErrCode(uint16 voltReading, uint16 tempReading) {
    uint8 ret = NONE;
    uint16 tempWarnThresh, tempErrThresh;
    
    if (voltReading > VOLT_MAX_WARN) {
        ret |= VOLT_HIGH | WARN_BIT;
        
        if (voltReading > VOLT_MAX_ERROR)
            ret |= ERROR_BIT;
    } else if (voltReading < VOLT_MIN_WARN){
        ret |= VOLT_LOW | WARN_BIT;
        
        if (voltReading < VOLT_MIN_ERROR)
            ret |= ERROR_BIT;
    }
        
    if (hallDischarge()) {
        tempWarnThresh = TEMP_DISCHARGE_MAX_WARN;
        tempErrThresh = TEMP_DISCHARGE_MAX_ERROR;
    } else {
        tempWarnThresh = TEMP_CHARGE_MAX_WARN;
        tempErrThresh = TEMP_CHARGE_MAX_ERROR;
    }
        
    if (tempReading > tempWarnThresh) {
        ret |= TEMP_HIGH | WARN_BIT;
        
        if (tempReading > tempErrThresh)
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

int CANBus_SendMessage(uint8 errCode, uint8 n) {
    uint8 out_data[3];
    
    if (errCode == NONE)
        return 0;
    
    out_data[0] = errCode;

    if (IS_CURRENT_CODE(errCode)) {
        // error code is current error
        out_data[1] = (uint8)(rawCurrentData >> 8); // MSByte
        out_data[2] = (uint8)rawCurrentData;        // LSByte
    } else {
        // error code is voltage/temperature error
        out_data[1] = n;
        
        if (IS_VOLT_CODE(errCode))
            out_data[2] = sensor[n].voltData >> 2;
        else if (IS_TEMP_CODE(errCode))
            out_data[2] = sensor[n].tempData >> 2;
    }
    
    return can_putd(tx_id,out_data,3,tx_pri,tx_ext,tx_rtr);
}

void setPacketData(int8 packet, uint8* data) {
    uint16 i;
    
    if(packet == 0) {
        data[0] = 0;
        
        for(i = 1; i < 8; i++)
            data[i] = (uint8)(sensor[i-1].voltData>>2);
    } else if (packet == 1) {
        data[0] = (uint8)(rawCurrentData>>8);
        for(i = 1; i < 8; i++)
            data[i] = (uint8)(sensor[i+6].voltData>>2);
    } else if (packet == 2) {
        data[0] = (uint8)rawCurrentData;
        for(i = 1; i < 8; i++)
            data[i] = (uint8)(sensor[i-1].tempData>>2);
    } else if (packet == 3) {
        for(i = 0; i < 7; i++)
            data[i] = (uint8)(sensor[i+7].tempData>>2);
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

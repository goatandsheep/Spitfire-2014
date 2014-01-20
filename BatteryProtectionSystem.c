/**McMaster University Solar Car Club
  *Fireball II January 20 2014
  *Battery Protection Team
  *-------------------------
  */

#include <18F26K80.h>
#include <can-18F4580_mscp.c>

#device adc = 16

#FUSES NOWDT
#FUSES HSH
#FUSES NOPLLEN
#FUSES NOBROWNOUT

#use delay(clock = 20E6);
#use rs232(baud = 112000, xmit = RS232_XMIT_PIN, rcv = RS232_RCV_PIN, parity = N, stream = debugs)
#use i2c(master, sda = I2C_SDA_PIN, scl = I2C_SCL_PIN);

#define RS232_XMIT_PIN PIN_C6
#define RS232_RCV_PIN PIN_C7
#define I2C_SDA_PIN PIN_C4
#define I2C_SCL_PIN PIN_C3
#define I2C_WRITE_BIT 0
#define I2C_READ_BIT 1
#define ADC_CHANNEL 1

typedef uint8 unsigned int8;
typedef uint16 unsigned int16;

const uint8 totalSensors = 14;
const uint8 sensorIDs[totalSensors] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0};

void main(void) {
   setup_comparator(NC_NC_NC_NC);
   setup_adc(ADC_CLOCK_DIV_32);
   setup_adc_ports(ALL_ANALOG);
   set_adc_channel(ADC_CHANNEL)
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

uint16 combineInts(uint8 high, uint8 low) {
    uint16 r = high;
    r = r << 3;
    low = low >> 5;
    r = r + low;
    
    return (r);
}

int rawCurrentRetrieve(void) {
    uint16 r;
    r = read_adc();
    //sensor result conversion pending
    
    return(r);
}

void LCD_Send(void) {
}

void CANBus_Send(void) {
}

/*      float_util.c
 *
 *  Provides utility functions for converting floating point numbers to and from
 *  raw binary representations, as well as converting from IEEE754 single
 *  precision floating point into microchip single precision floating point.
 *
 *  For more information on microchip's floating point format, see 
 *  Documentation.txt
 *
 *  These functions could probably be heavily optimized by using PIC assembly.
 *  This is left as an excersize for someone with more time than me.
 *
 */

/* floatToRaw()
 *
 * raw - four bytes, set to raw value of f
 * f   - source floating point number
 */
void floatToRaw(unsigned int8 *raw, float f) {
    memcpy(raw, &f, 4);
}

/* rawToFloat()
 *
 * raw - source bytes
 *
 * Returns:
 * Floating point number from raw data
 */
float rawToFloat(unsigned int8 *raw) {
    float f;
    memcpy(&f, raw, 4);
    return f;
}

/* IEEERawToRaw()
 *
 * raw - four bytes, converted from IEEE754 representation into microchip
 *          floating point representation
 */
void IEEERawToRaw(unsigned int8 *IEEEraw, unsigned int8 *raw) {
    int1 sign =  bit_test(IEEEraw[3], 7);
    int1 lsb = bit_test(IEEEraw[2], 7);
    
    raw[0] = IEEEraw[3];
    raw[1] = IEEEraw[2];
    raw[2] = IEEEraw[1];
    raw[3] = IEEEraw[0];

    raw[0] = (raw[0] << 1) | lsb;
    raw[1] = (raw[1]&0x7F) | (sign << 7);
}

/* IEEERawToFloat()
 *
 * raw - four bytes, set to microchip representation of f
 * f   - floating point number in IEEE754 format
 *
 * PIC uses the microchip floating point representation internally, so it
 * doesn't make sense to store the raw value any other way.
 */
float IEEERawToFloat(unsigned int8 *raw) {
    unsigned int8 temp[4];
    float f;

    // Swap sign and exponent
    IEEERawToRaw(raw, temp);
    
    // Load raw binary
    memcpy(&f, &temp, 4);
    
    return f;
}

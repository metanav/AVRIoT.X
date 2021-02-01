#include <math.h>
#include "i2c_simple_master.h"
#include "ADXL345.h"

#define ADXL345_DEVICE (0x53)    // ADXL345 device address
#define ADXL345_TO_READ (6)      // num of bytes we are going to read each time (two bytes for each axis)

void ADXL345_init() {
    status = ADXL345_OK;
    error_code = ADXL345_NO_ERROR;

    gains[0] = 0.00376390;
    gains[1] = 0.00376009;
    gains[2] = 0.00349265;
    //set activity/ inactivity thresholds (0-255)
    ADXL345_setActivityThreshold(75); //62.5mg per increment
    ADXL345_setInactivityThreshold(75); //62.5mg per increment
    ADXL345_setTimeInactivity(10); // how many seconds of no activity is inactive?

    //look of activity movement on this axes - 1 == on; 0 == off
    ADXL345_setActivityX(1);
    ADXL345_setActivityY(1);
    ADXL345_setActivityZ(1);

    //look of inactivity movement on this axes - 1 == on; 0 == off
    ADXL345_setInactivityX(1);
    ADXL345_setInactivityY(1);
    ADXL345_setInactivityZ(1);

    //look of tap movement on this axes - 1 == on; 0 == off
    ADXL345_setTapDetectionOnX(0);
    ADXL345_setTapDetectionOnY(0);
    ADXL345_setTapDetectionOnZ(1);

    //set values for what is a tap, and what is a double tap (0-255)
    ADXL345_setTapThreshold(50); //62.5mg per increment
    ADXL345_setTapDuration(15); //625us per increment
    ADXL345_setDoubleTapLatency(80); //1.25ms per increment
    ADXL345_setDoubleTapWindow(200); //1.25ms per increment

    //set values for what is considered freefall (0-255)
    ADXL345_setFreeFallThreshold(7); //(5 - 9) recommended - 62.5mg per increment
    ADXL345_setFreeFallDuration(45); //(20 - 70) recommended - 5ms per increment

    //setting all interrupts to take place on int pin 1
    //I had issues with int pin 2, was unable to reset it
    ADXL345_setInterruptMapping(ADXL345_INT_SINGLE_TAP_BIT,   ADXL345_INT1_PIN);
    ADXL345_setInterruptMapping(ADXL345_INT_DOUBLE_TAP_BIT,   ADXL345_INT1_PIN);
    ADXL345_setInterruptMapping(ADXL345_INT_FREE_FALL_BIT,    ADXL345_INT1_PIN);
    ADXL345_setInterruptMapping(ADXL345_INT_ACTIVITY_BIT,     ADXL345_INT1_PIN);
    ADXL345_setInterruptMapping(ADXL345_INT_INACTIVITY_BIT,   ADXL345_INT1_PIN);

    //register interrupt actions - 1 == on; 0 == off
    ADXL345_setInterrupt(ADXL345_INT_SINGLE_TAP_BIT, 1);
    ADXL345_setInterrupt(ADXL345_INT_DOUBLE_TAP_BIT, 1);
    ADXL345_setInterrupt(ADXL345_INT_FREE_FALL_BIT,  1);
    ADXL345_setInterrupt(ADXL345_INT_ACTIVITY_BIT,   1);
    ADXL345_setInterrupt(ADXL345_INT_INACTIVITY_BIT, 1);
    
}

void ADXL345_powerOn() {
    //Turning on the ADXL345
    ADXL345_writeTo(ADXL345_POWER_CTL, 0);
    ADXL345_writeTo(ADXL345_POWER_CTL, 16);
    ADXL345_writeTo(ADXL345_POWER_CTL, 8);
}

// Reads the acceleration into three variable x, y and z
void ADXL345_readAccel(int* xyz) {
    ADXL345_readXYZ(xyz, xyz + 1, xyz + 2);
}
void ADXL345_readXYZ(int* x, int* y, int* z) {
    ADXL345_readFrom(ADXL345_DATAX0, ADXL345_TO_READ, _buff); //read the acceleration data from the ADXL345
    *x = (short)((((unsigned short)_buff[1]) << 8) | _buff[0]);
    *y = (short)((((unsigned short)_buff[3]) << 8) | _buff[2]);
    *z = (short)((((unsigned short)_buff[5]) << 8) | _buff[4]);
}

void ADXL345_getAcceleration(double* xyz) {
    int i;
    int xyz_int[3];
    ADXL345_readAccel(xyz_int);
    for (i = 0; i < 3; i++) {
        xyz[i] = xyz_int[i] * gains[i] * 9.81f * 100.0;
    }
}
// Writes val to address register on device
void ADXL345_writeTo(uint8_t address, uint8_t val) {
    i2c_write1ByteRegister(ADXL345_DEVICE, address, val);
}

// Reads num bytes starting from address register on device in to _buff array
void ADXL345_readFrom(uint8_t address, int num, uint8_t *_buff) {
    i2c_readDataBlock(ADXL345_DEVICE,  address, _buff, num);

}

// Gets the range setting and return it into rangeSetting
// it can be 2, 4, 8 or 16
void ADXL345_getRangeSetting(uint8_t* rangeSetting) {
    uint8_t _b;
    ADXL345_readFrom(ADXL345_DATA_FORMAT, 1, &_b);
    *rangeSetting = _b & 0b00000011;
}

// Sets the range setting, possible values are: 2, 4, 8, 16
void ADXL345_setRangeSetting(int val) {
    uint8_t _s;
    uint8_t _b;

    switch (val) {
        case 2:
            _s = 0b00000000;
            break;
        case 4:
            _s = 0b00000001;
            break;
        case 8:
            _s = 0b00000010;
            break;
        case 16:
            _s = 0b00000011;
            break;
        default:
            _s = 0b00000000;
    }
    ADXL345_readFrom(ADXL345_DATA_FORMAT, 1, &_b);
    _s |= (_b & 0b11101100);
    ADXL345_writeTo(ADXL345_DATA_FORMAT, _s);
}
// gets the state of the SELF_TEST bit
bool ADXL345_getSelfTestBit() {
    return ADXL345_getRegisterBit(ADXL345_DATA_FORMAT, 7);
}

// Sets the SELF-TEST bit
// if set to 1 it applies a self-test force to the sensor causing a shift in the output data
// if set to 0 it disables the self-test force
void ADXL345_setSelfTestBit(bool selfTestBit) {
    ADXL345_setRegisterBit(ADXL345_DATA_FORMAT, 7, selfTestBit);
}


// Gets the state of the INT_INVERT bit
bool ADXL345_getInterruptLevelBit() {
    return ADXL345_getRegisterBit(ADXL345_DATA_FORMAT, 5);
}

// Sets the INT_INVERT bit
// if set to 0 sets the interrupts to active high
// if set to 1 sets the interrupts to active low
void ADXL345_setInterruptLevelBit(bool interruptLevelBit) {
    ADXL345_setRegisterBit(ADXL345_DATA_FORMAT, 5, interruptLevelBit);
}

// Gets the state of the FULL_RES bit
bool ADXL345_getFullResBit() {
    return ADXL345_getRegisterBit(ADXL345_DATA_FORMAT, 3);
}

// Sets the FULL_RES bit
// if set to 1, the device is in full resolution mode, where the output resolution increases with the
//   g range set by the range bits to maintain a 4mg/LSB scal factor
// if set to 0, the device is in 10-bit mode, and the range buts determine the maximum g range
//   and scale factor
void ADXL345_setFullResBit(bool fullResBit) {
    ADXL345_setRegisterBit(ADXL345_DATA_FORMAT, 3, fullResBit);
}

// Gets the state of the justify bit
bool ADXL345_getJustifyBit() {
    return ADXL345_getRegisterBit(ADXL345_DATA_FORMAT, 2);
}

// Sets the JUSTIFY bit
// if sets to 1 selects the left justified mode
// if sets to 0 selects right justified mode with sign extension
void ADXL345_setJustifyBit(bool justifyBit) {
    ADXL345_setRegisterBit(ADXL345_DATA_FORMAT, 2, justifyBit);
}

// Sets the THRESH_TAP byte value
// it should be between 0 and 255
// the scale factor is 62.5 mg/LSB
// A value of 0 may result in undesirable behavior
void ADXL345_setTapThreshold(int tapThreshold) {
    tapThreshold = constrain(tapThreshold, 0, 255);
    uint8_t _b = (uint8_t)tapThreshold;
    ADXL345_writeTo(ADXL345_THRESH_TAP, _b);
}

// Gets the THRESH_TAP byte value
// return value is comprised between 0 and 255
// the scale factor is 62.5 mg/LSB
int ADXL345_getTapThreshold() {
    uint8_t _b;
    ADXL345_readFrom(ADXL345_THRESH_TAP, 1, &_b);
    return (int) _b;
}

// set/get the gain for each axis in Gs / count
void ADXL345_setAxisGains(double* _gains) {
    int i;
    for (i = 0; i < 3; i++) {
        gains[i] = _gains[i];
    }
}
void ADXL345_getAxisGains(double* _gains) {
    int i;
    for (i = 0; i < 3; i++) {
        _gains[i] = gains[i];
    }
}


// Sets the OFSX, OFSY and OFSZ bytes
// OFSX, OFSY and OFSZ are user offset adjustments in twos complement format with
// a scale factor of 15,6mg/LSB
// OFSX, OFSY and OFSZ should be comprised between
void ADXL345_setAxisOffset(int x, int y, int z) {
    ADXL345_writeTo(ADXL345_OFSX, (uint8_t)x);
    ADXL345_writeTo(ADXL345_OFSY, (uint8_t)y);
    ADXL345_writeTo(ADXL345_OFSZ, (uint8_t)z);
}

// Gets the OFSX, OFSY and OFSZ bytes
void ADXL345_getAxisOffset(int* x, int* y, int* z) {
    uint8_t _b;
    ADXL345_readFrom(ADXL345_OFSX, 1, &_b);
    *x = (int) _b;
    ADXL345_readFrom(ADXL345_OFSY, 1, &_b);
    *y = (int) _b;
    ADXL345_readFrom(ADXL345_OFSZ, 1, &_b);
    *z = (int) _b;
}

// Sets the DUR byte
// The DUR byte contains an unsigned time value representing the maximum time
// that an event must be above THRESH_TAP threshold to qualify as a tap event
// The scale factor is 625µs/LSB
// A value of 0 disables the tap/double tap funcitons. Max value is 255.
void ADXL345_setTapDuration(int tapDuration) {
    tapDuration = constrain(tapDuration, 0, 255);
    uint8_t _b = (uint8_t)tapDuration;
    ADXL345_writeTo(ADXL345_DUR, _b);
}

// Gets the DUR byte
int ADXL345_getTapDuration() {
    uint8_t _b;
    ADXL345_readFrom(ADXL345_DUR, 1, &_b);
    return(int) _b;
}

// Sets the latency (latent register) which contains an unsigned time value
// representing the wait time from the detection of a tap event to the start
// of the time window, during which a possible second tap can be detected.
// The scale factor is 1.25ms/LSB. A value of 0 disables the double tap function.
// It accepts a maximum value of 255.
void ADXL345_setDoubleTapLatency(int doubleTapLatency) {
    uint8_t _b = (uint8_t)doubleTapLatency;
    ADXL345_writeTo(ADXL345_LATENT, _b);
}

// Gets the Latent value
int ADXL345_getDoubleTapLatency() {
    uint8_t _b;
    ADXL345_readFrom(ADXL345_LATENT, 1, &_b);
    return (int) _b;
}

// Sets the Window register, which contains an unsigned time value representing
// the amount of time after the expiration of the latency time (Latent register)
// during which a second valud tap can begin. The scale factor is 1.25ms/LSB. A
// value of 0 disables the double tap function. The maximum value is 255.
void ADXL345_setDoubleTapWindow(int doubleTapWindow) {
    doubleTapWindow = constrain(doubleTapWindow, 0, 255);
    uint8_t _b = (uint8_t)doubleTapWindow;
    ADXL345_writeTo(ADXL345_WINDOW, _b);
}

// Gets the Window register
int ADXL345_getDoubleTapWindow() {
    uint8_t _b;
    ADXL345_readFrom(ADXL345_WINDOW, 1, &_b);
    return (int) _b;
}

// Sets the THRESH_ACT byte which holds the threshold value for detecting activity.
// The data format is unsigned, so the magnitude of the activity event is compared
// with the value is compared with the value in the THRESH_ACT register. The scale
// factor is 62.5mg/LSB. A value of 0 may result in undesirable behavior if the
// activity interrupt is enabled. The maximum value is 255.
void ADXL345_setActivityThreshold(int activityThreshold) {
    activityThreshold = constrain(activityThreshold, 0, 255);
    uint8_t _b = (uint8_t)activityThreshold;
    ADXL345_writeTo(ADXL345_THRESH_ACT, _b);
}

// Gets the THRESH_ACT byte
int ADXL345_getActivityThreshold() {
    uint8_t _b;
    ADXL345_readFrom(ADXL345_THRESH_ACT, 1, &_b);
    return (int) _b;
}

// Sets the THRESH_INACT byte which holds the threshold value for detecting inactivity.
// The data format is unsigned, so the magnitude of the inactivity event is compared
// with the value is compared with the value in the THRESH_INACT register. The scale
// factor is 62.5mg/LSB. A value of 0 may result in undesirable behavior if the
// inactivity interrupt is enabled. The maximum value is 255.
void ADXL345_setInactivityThreshold(int inactivityThreshold) {
    inactivityThreshold = constrain(inactivityThreshold, 0, 255);
    uint8_t _b = (uint8_t)inactivityThreshold;
    ADXL345_writeTo(ADXL345_THRESH_INACT, _b);
}

// Gets the THRESH_INACT byte
int ADXL345_getInactivityThreshold() {
    uint8_t _b;
    ADXL345_readFrom(ADXL345_THRESH_INACT, 1, &_b);
    return (int) _b;
}

// Sets the TIME_INACT register, which contains an unsigned time value representing the
// amount of time that acceleration must be less thant the value in the THRESH_INACT
// register for inactivity to be declared. The scale factor is 1sec/LSB. The value must
// be between 0 and 255.
void ADXL345_setTimeInactivity(int timeInactivity) {
    timeInactivity = constrain(timeInactivity, 0, 255);
    uint8_t _b = (uint8_t)timeInactivity;
    ADXL345_writeTo(ADXL345_TIME_INACT, _b);
}

// Gets the TIME_INACT register
int ADXL345_getTimeInactivity() {
    uint8_t _b;
    ADXL345_readFrom(ADXL345_TIME_INACT, 1, &_b);
    return (int) _b;
}

// Sets the THRESH_FF register which holds the threshold value, in an unsigned format, for
// free-fall detection. The root-sum-square (RSS) value of all axes is calculated and
// compared whith the value in THRESH_FF to determine if a free-fall event occured. The
// scale factor is 62.5mg/LSB. A value of 0 may result in undesirable behavior if the free-fall
// interrupt is enabled. The maximum value is 255.
void ADXL345_setFreeFallThreshold(int freeFallThreshold) {
    freeFallThreshold = constrain(freeFallThreshold, 0, 255);
    uint8_t _b = (uint8_t)freeFallThreshold;
    ADXL345_writeTo(ADXL345_THRESH_FF, _b);
}

// Gets the THRESH_FF register.
int ADXL345_getFreeFallThreshold() {
    uint8_t _b;
    ADXL345_readFrom(ADXL345_THRESH_FF, 1, &_b);
    return (int) _b;
}

// Sets the TIME_FF register, which holds an unsigned time value representing the minimum
// time that the RSS value of all axes must be less than THRESH_FF to generate a free-fall
// interrupt. The scale factor is 5ms/LSB. A value of 0 may result in undesirable behavior if
// the free-fall interrupt is enabled. The maximum value is 255.
void ADXL345_setFreeFallDuration(int freeFallDuration) {
    freeFallDuration = constrain(freeFallDuration, 0, 255);
    uint8_t _b = (uint8_t)freeFallDuration;
    ADXL345_writeTo(ADXL345_TIME_FF, _b);
}

// Gets the TIME_FF register.
int ADXL345_getFreeFallDuration() {
    uint8_t _b;
    ADXL345_readFrom(ADXL345_TIME_FF, 1, &_b);
    return (int) _b;
}

bool ADXL345_isActivityXEnabled() {
    return ADXL345_getRegisterBit(ADXL345_ACT_INACT_CTL, 6);
}
bool ADXL345_isActivityYEnabled() {
    return ADXL345_getRegisterBit(ADXL345_ACT_INACT_CTL, 5);
}
bool ADXL345_isActivityZEnabled() {
    return ADXL345_getRegisterBit(ADXL345_ACT_INACT_CTL, 4);
}
bool ADXL345_isInactivityXEnabled() {
    return ADXL345_getRegisterBit(ADXL345_ACT_INACT_CTL, 2);
}
bool ADXL345_isInactivityYEnabled() {
    return ADXL345_getRegisterBit(ADXL345_ACT_INACT_CTL, 1);
}
bool ADXL345_isInactivityZEnabled() {
    return ADXL345_getRegisterBit(ADXL345_ACT_INACT_CTL, 0);
}

void ADXL345_setActivityX(bool state) {
    ADXL345_setRegisterBit(ADXL345_ACT_INACT_CTL, 6, state);
}
void ADXL345_setActivityY(bool state) {
    ADXL345_setRegisterBit(ADXL345_ACT_INACT_CTL, 5, state);
}
void ADXL345_setActivityZ(bool state) {
    ADXL345_setRegisterBit(ADXL345_ACT_INACT_CTL, 4, state);
}
void ADXL345_setInactivityX(bool state) {
    ADXL345_setRegisterBit(ADXL345_ACT_INACT_CTL, 2, state);
}
void ADXL345_setInactivityY(bool state) {
    ADXL345_setRegisterBit(ADXL345_ACT_INACT_CTL, 1, state);
}
void ADXL345_setInactivityZ(bool state) {
    ADXL345_setRegisterBit(ADXL345_ACT_INACT_CTL, 0, state);
}

bool ADXL345_isActivityAc() {
    return ADXL345_getRegisterBit(ADXL345_ACT_INACT_CTL, 7);
}
bool ADXL345_isInactivityAc() {
    return ADXL345_getRegisterBit(ADXL345_ACT_INACT_CTL, 3);
}

void ADXL345_setActivityAc(bool state) {
    ADXL345_setRegisterBit(ADXL345_ACT_INACT_CTL, 7, state);
}
void ADXL345_setInactivityAc(bool state) {
    ADXL345_setRegisterBit(ADXL345_ACT_INACT_CTL, 3, state);
}

bool ADXL345_getSuppressBit() {
    return ADXL345_getRegisterBit(ADXL345_TAP_AXES, 3);
}
void ADXL345_setSuppressBit(bool state) {
    ADXL345_setRegisterBit(ADXL345_TAP_AXES, 3, state);
}

bool ADXL345_isTapDetectionOnX() {
    return ADXL345_getRegisterBit(ADXL345_TAP_AXES, 2);
}
void ADXL345_setTapDetectionOnX(bool state) {
    ADXL345_setRegisterBit(ADXL345_TAP_AXES, 2, state);
}
bool ADXL345_isTapDetectionOnY() {
    return ADXL345_getRegisterBit(ADXL345_TAP_AXES, 1);
}
void ADXL345_setTapDetectionOnY(bool state) {
    ADXL345_setRegisterBit(ADXL345_TAP_AXES, 1, state);
}
bool ADXL345_isTapDetectionOnZ() {
    return ADXL345_getRegisterBit(ADXL345_TAP_AXES, 0);
}
void ADXL345_setTapDetectionOnZ(bool state) {
    ADXL345_setRegisterBit(ADXL345_TAP_AXES, 0, state);
}

bool ADXL345_isActivitySourceOnX() {
    return ADXL345_getRegisterBit(ADXL345_ACT_TAP_STATUS, 6);
}
bool ADXL345_isActivitySourceOnY() {
    return ADXL345_getRegisterBit(ADXL345_ACT_TAP_STATUS, 5);
}
bool ADXL345_isActivitySourceOnZ() {
    return ADXL345_getRegisterBit(ADXL345_ACT_TAP_STATUS, 4);
}

bool ADXL345_isTapSourceOnX() {
    return ADXL345_getRegisterBit(ADXL345_ACT_TAP_STATUS, 2);
}
bool ADXL345_isTapSourceOnY() {
    return ADXL345_getRegisterBit(ADXL345_ACT_TAP_STATUS, 1);
}
bool ADXL345_isTapSourceOnZ() {
    return ADXL345_getRegisterBit(ADXL345_ACT_TAP_STATUS, 0);
}

bool ADXL345_isAsleep() {
    return ADXL345_getRegisterBit(ADXL345_ACT_TAP_STATUS, 3);
}

bool ADXL345_isLowPower() {
    return ADXL345_getRegisterBit(ADXL345_BW_RATE, 4);
}
void ADXL345_setLowPower(bool state) {
    ADXL345_setRegisterBit(ADXL345_BW_RATE, 4, state);
}

double ADXL345_getRate() {
    uint8_t _b;
    ADXL345_readFrom(ADXL345_BW_RATE, 1, &_b);

    _b &= 0b00001111;
    return (pow(2, ((int) _b) - 6)) * 6.25;
}

void ADXL345_setRate(double rate) {
    uint8_t _b, _s;
    int v = (int)(rate / 6.25);
    int r = 0;
    while (v >>= 1) {
        r++;
    }
    if (r <= 9) {
        ADXL345_readFrom(ADXL345_BW_RATE, 1, &_b);
        _s = (uint8_t)(r + 6) | (_b & 0b11110000);
        ADXL345_writeTo(ADXL345_BW_RATE, _s);
    }
}

void ADXL345_set_bw(uint8_t bw_code) {
    if ((bw_code < ADXL345_BW_3) || (bw_code > ADXL345_BW_1600)) {
        status = false;
        error_code = ADXL345_BAD_ARG;
    } else {
        ADXL345_writeTo(ADXL345_BW_RATE, bw_code);
    }
}

uint8_t ADXL345_get_bw_code() {
    uint8_t bw_code;
    ADXL345_readFrom(ADXL345_BW_RATE, 1, &bw_code);
    return bw_code;
}


//Used to check if action was triggered in interrupts
//Example triggered(interrupts, ADXL345_SINGLE_TAP);
bool ADXL345_triggered(uint8_t interrupts, int mask) {
    return ((interrupts >> mask) & 1);
}


/*
    ADXL345_DATA_READY
    ADXL345_SINGLE_TAP
    ADXL345_DOUBLE_TAP
    ADXL345_ACTIVITY
    ADXL345_INACTIVITY
    ADXL345_FREE_FALL
    ADXL345_WATERMARK
    ADXL345_OVERRUNY
*/





uint8_t ADXL345_getInterruptSource() {
    uint8_t _b;
    ADXL345_readFrom(ADXL345_INT_SOURCE, 1, &_b);
    return _b;
}

//bool ADXL345_getInterruptSource(uint8_t interruptBit) {
//    return ADXL345_getRegisterBit(ADXL345_INT_SOURCE, interruptBit);
//}

bool ADXL345_getInterruptMapping(uint8_t interruptBit) {
    return ADXL345_getRegisterBit(ADXL345_INT_MAP, interruptBit);
}

// Set the mapping of an interrupt to pin1 or pin2
// eg: setInterruptMapping(ADXL345_INT_DOUBLE_TAP_BIT,ADXL345_INT2_PIN);
void ADXL345_setInterruptMapping(uint8_t interruptBit, bool interruptPin) {
    ADXL345_setRegisterBit(ADXL345_INT_MAP, interruptBit, interruptPin);
}

bool ADXL345_isInterruptEnabled(uint8_t interruptBit) {
    return ADXL345_getRegisterBit(ADXL345_INT_ENABLE, interruptBit);
}

void ADXL345_setInterrupt(uint8_t interruptBit, bool state) {
    ADXL345_setRegisterBit(ADXL345_INT_ENABLE, interruptBit, state);
}

void ADXL345_setRegisterBit(uint8_t regAdress, int bitPos, bool state) {
    uint8_t _b;
    ADXL345_readFrom(regAdress, 1, &_b);
    if (state) {
        _b |= (1 << bitPos);  // forces nth bit of _b to be 1.  all other bits left alone.
    } else {
        _b &= ~(1 << bitPos); // forces nth bit of _b to be 0.  all other bits left alone.
    }
    ADXL345_writeTo(regAdress, _b);
}



bool ADXL345_getRegisterBit(uint8_t regAdress, int bitPos) {
    uint8_t _b;
    ADXL345_readFrom(regAdress, 1, &_b);
    return ((_b >> bitPos) & 1);
}

// print all register value to the serial ouptut, which requires it to be setup
// this can be used to manually to check the current configuration of the device
void ADXL345_printAllRegister() {
    uint8_t _b;
    printf("0x00: ");
    ADXL345_readFrom(0x00, 1, &_b);
    print_byte(_b);
    printf("\n");
    int i;
    for (i = 29; i <= 57; i++) {
        printf("0x");
        printf("%X",i);
        printf(": ");
        ADXL345_readFrom(i, 1, &_b);
        print_byte(_b);
        printf("\n");
    }
}

// set the operation mode

void ADXL345_setMode(uint8_t operationMode) {
    uint8_t _b;
    ADXL345_readFrom(ADXL345_FIFO_CTL, 1, &_b);
    _b &= ~(0b11000000); //clearing bit 6 and 7
    _b |= (operationMode << 6); //setting op mode


    //setRegisterBit(ADXL345_FIFO_CTL, 6, operationMode);
    ADXL345_writeTo(ADXL345_FIFO_CTL, _b);
}
// readback mode

uint8_t ADXL345_getMode(void) {
    uint8_t _b;
    ADXL345_readFrom(ADXL345_FIFO_CTL, 1, &_b);
    _b &= 0b11000000;  //masking bit 6 and 7
    _b = (_b >> 6); //setting op mode
    return _b;
}

// set watermark

void ADXL345_setWatermark(uint8_t watermark) {
    uint8_t _b, _w;

    ADXL345_readFrom(ADXL345_FIFO_CTL, 1, &_b);
    _b &= (0b11100000); //clearing bit 0 to 4
    _w = watermark & (0b00011111);  //clearing highest 3 bits in waterlevel
    _b |= _w; //setting waterlevel in operationmode register
    //setRegisterBit(ADXL345_FIFO_CTL, 6, operationMode);
    ADXL345_writeTo(ADXL345_FIFO_CTL, _b);
}

// read how many samples in Fifi

uint8_t ADXL345_getFifoEntries(void) {
    uint8_t _b;
    ADXL345_readFrom(ADXL345_FIFO_STATUS, 1, &_b);
    _b &=  0b00111111;

    return _b;
}

void ADXL345_burstReadXYZ(int* x, int* y, int* z, uint8_t samples) {
    for (int i = 0; i < samples; i++) {
        ADXL345_readFrom(ADXL345_DATAX0, ADXL345_TO_READ, _buff); //read the acceleration data from the ADXL345
        x[i] = (short)((((unsigned short)_buff[1]) << 8) | _buff[0]);
        y[i] = (short)((((unsigned short)_buff[3]) << 8) | _buff[2]);
        z[i] = (short)((((unsigned short)_buff[5]) << 8) | _buff[4]);
    }
}

int constrain(int x, int a, int b) {
    if(x < a) {
        return a;
    }
    else if(b < x) {
        return b;
    }
    else 
        return x;
}


void print_byte(uint8_t val) {
    int i;
    printf("B");
    for (i = 7; i >= 0; i--) {
        printf("%u", val >> i & 1);
    }
}

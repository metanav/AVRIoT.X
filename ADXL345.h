#ifndef ADXL345_h
#define ADXL345_h

#include <stdbool.h>

/* ------- Register names ------- */
#define ADXL345_DEVID 0x00
#define ADXL345_RESERVED1 0x01
#define ADXL345_THRESH_TAP 0x1d
#define ADXL345_OFSX 0x1e
#define ADXL345_OFSY 0x1f
#define ADXL345_OFSZ 0x20
#define ADXL345_DUR 0x21
#define ADXL345_LATENT 0x22
#define ADXL345_WINDOW 0x23
#define ADXL345_THRESH_ACT 0x24
#define ADXL345_THRESH_INACT 0x25
#define ADXL345_TIME_INACT 0x26
#define ADXL345_ACT_INACT_CTL 0x27
#define ADXL345_THRESH_FF 0x28
#define ADXL345_TIME_FF 0x29
#define ADXL345_TAP_AXES 0x2a
#define ADXL345_ACT_TAP_STATUS 0x2b
#define ADXL345_BW_RATE 0x2c
#define ADXL345_POWER_CTL 0x2d
#define ADXL345_INT_ENABLE 0x2e
#define ADXL345_INT_MAP 0x2f
#define ADXL345_INT_SOURCE 0x30
#define ADXL345_DATA_FORMAT 0x31
#define ADXL345_DATAX0 0x32
#define ADXL345_DATAX1 0x33
#define ADXL345_DATAY0 0x34
#define ADXL345_DATAY1 0x35
#define ADXL345_DATAZ0 0x36
#define ADXL345_DATAZ1 0x37
#define ADXL345_FIFO_CTL 0x38
#define ADXL345_FIFO_STATUS 0x39

#define ADXL345_BW_1600 0xF // 1111
#define ADXL345_BW_800  0xE // 1110
#define ADXL345_BW_400  0xD // 1101  
#define ADXL345_BW_200  0xC // 1100
#define ADXL345_BW_100  0xB // 1011  
#define ADXL345_BW_50   0xA // 1010 
#define ADXL345_BW_25   0x9 // 1001 
#define ADXL345_BW_12   0x8 // 1000 
#define ADXL345_BW_6    0x7 // 0111
#define ADXL345_BW_3    0x6 // 0110


/*
    Interrupt PINs
    INT1: 0
    INT2: 1
*/
#define ADXL345_INT1_PIN 0x00
#define ADXL345_INT2_PIN 0x01

/*Interrupt bit position*/
#define ADXL345_INT_DATA_READY_BIT 0x07
#define ADXL345_INT_SINGLE_TAP_BIT 0x06
#define ADXL345_INT_DOUBLE_TAP_BIT 0x05
#define ADXL345_INT_ACTIVITY_BIT   0x04
#define ADXL345_INT_INACTIVITY_BIT 0x03
#define ADXL345_INT_FREE_FALL_BIT  0x02
#define ADXL345_INT_WATERMARK_BIT  0x01
#define ADXL345_INT_OVERRUNY_BIT   0x00

#define ADXL345_DATA_READY 0x07
#define ADXL345_SINGLE_TAP 0x06
#define ADXL345_DOUBLE_TAP 0x05
#define ADXL345_ACTIVITY   0x04
#define ADXL345_INACTIVITY 0x03
#define ADXL345_FREE_FALL  0x02
#define ADXL345_WATERMARK  0x01
#define ADXL345_OVERRUNY   0x00

#define ADXL345_OK    1 // no error
#define ADXL345_ERROR 0 // indicates error is predent

#define ADXL345_NO_ERROR   0 // initial state
#define ADXL345_READ_ERROR 1 // problem reading accel
#define ADXL345_BAD_ARG    2 // bad method argument

#define ADXL345_MODE_BYPASS 0x00 	//0000
#define ADXL345_MODE_FIFO 0x01 		//0001
#define ADXL345_MODE_STREM 0x02 	//0010
#define ADXL345_MODE_TRIGGER 0x03 	//0011


bool status;           // set when error occurs
// see error code for details
uint8_t error_code;       // Initial state
double gains[3];        // counts to Gs

void ADXL345_init();
void ADXL345_powerOn();
void ADXL345_readAccel(int* xyx);
void ADXL345_readXYZ(int* x, int* y, int* z);
void ADXL345_getAcceleration(double* xyz);

void ADXL345_setTapThreshold(int tapThreshold);
int ADXL345_getTapThreshold();
void ADXL345_setAxisGains(double* _gains);
void ADXL345_getAxisGains(double* _gains);
void ADXL345_setAxisOffset(int x, int y, int z);
void ADXL345_getAxisOffset(int* x, int* y, int* z);
void ADXL345_setTapDuration(int tapDuration);
int  ADXL345_getTapDuration();
void ADXL345_setDoubleTapLatency(int doubleTapLatency);
int  ADXL345_getDoubleTapLatency();
void ADXL345_setDoubleTapWindow(int doubleTapWindow);
int  ADXL345_getDoubleTapWindow();
void ADXL345_setActivityThreshold(int activityThreshold);
int  ADXL345_getActivityThreshold();
void ADXL345_setInactivityThreshold(int inactivityThreshold);
int  ADXL345_getInactivityThreshold();
void ADXL345_setTimeInactivity(int timeInactivity);
int  ADXL345_getTimeInactivity();
void ADXL345_setFreeFallThreshold(int freeFallthreshold);
int  ADXL345_getFreeFallThreshold();
void ADXL345_setFreeFallDuration(int freeFallDuration);
int  ADXL345_getFreeFallDuration();

bool ADXL345_isActivityXEnabled();
bool ADXL345_isActivityYEnabled();
bool ADXL345_isActivityZEnabled();
bool ADXL345_isInactivityXEnabled();
bool ADXL345_isInactivityYEnabled();
bool ADXL345_isInactivityZEnabled();
bool ADXL345_isActivityAc();
bool ADXL345_isInactivityAc();
void ADXL345_setActivityAc(bool state);
void ADXL345_setInactivityAc(bool state);

bool ADXL345_getSuppressBit();
void ADXL345_setSuppressBit(bool state);
bool ADXL345_isTapDetectionOnX();
void ADXL345_setTapDetectionOnX(bool state);
bool ADXL345_isTapDetectionOnY();
void ADXL345_setTapDetectionOnY(bool state);
bool ADXL345_isTapDetectionOnZ();
void ADXL345_setTapDetectionOnZ(bool state);

void ADXL345_setActivityX(bool state);
void ADXL345_setActivityY(bool state);
void ADXL345_setActivityZ(bool state);
void ADXL345_setInactivityX(bool state);
void ADXL345_setInactivityY(bool state);
void ADXL345_setInactivityZ(bool state);

bool ADXL345_isActivitySourceOnX();
bool ADXL345_isActivitySourceOnY();
bool ADXL345_isActivitySourceOnZ();
bool ADXL345_isTapSourceOnX();
bool ADXL345_isTapSourceOnY();
bool ADXL345_isTapSourceOnZ();
bool ADXL345_isAsleep();

bool ADXL345_isLowPower();
void ADXL345_setLowPower(bool state);
double ADXL345_getRate();
void ADXL345_setRate(double rate);
void ADXL345_set_bw(uint8_t bw_code);
uint8_t ADXL345_get_bw_code();


bool ADXL345_triggered(uint8_t interrupts, int mask);


uint8_t ADXL345_getInterruptSource();
//bool ADXL345_getInterruptSource(uint8_t interruptBit);
bool ADXL345_getInterruptMapping(uint8_t interruptBit);
void ADXL345_setInterruptMapping(uint8_t interruptBit, bool interruptPin);
bool ADXL345_isInterruptEnabled(uint8_t interruptBit);
void ADXL345_setInterrupt(uint8_t interruptBit, bool state);


void ADXL345_setMode(uint8_t modeBit); // setting operation mode
uint8_t ADXL345_getMode();
void ADXL345_setWatermark(uint8_t watermark);
uint8_t ADXL345_getFifoEntries(void);		// reading number of samples from fifo
void ADXL345_burstReadXYZ(int* x, int* y, int* z, uint8_t samples);  // burst read function for getting all samples from fifo

void ADXL345_getRangeSetting(uint8_t* rangeSetting);
void ADXL345_setRangeSetting(int val);
bool ADXL345_getSelfTestBit();
void ADXL345_setSelfTestBit(bool selfTestBit);
bool ADXL345_getSpiBit();
void ADXL345_setSpiBit(bool spiBit);
bool ADXL345_getInterruptLevelBit();
void ADXL345_setInterruptLevelBit(bool interruptLevelBit);
bool ADXL345_getFullResBit();
void ADXL345_setFullResBit(bool fullResBit);
bool ADXL345_getJustifyBit();
void ADXL345_setJustifyBit(bool justifyBit);
void ADXL345_printAllRegister();

void ADXL345_writeTo(uint8_t address, uint8_t val);
void ADXL345_readFrom(uint8_t address, int num, uint8_t buff[]);
void ADXL345_setRegisterBit(uint8_t regAdress, int bitPos, bool state);
bool ADXL345_getRegisterBit(uint8_t regAdress, int bitPos);
uint8_t _buff[6] ;    //6 uint8_ts buffer for saving data read from the device
void print_byte(uint8_t val);
int constrain(int x, int a, int b);
#endif

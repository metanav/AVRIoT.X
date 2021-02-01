/* 
 * File:   MAX30105.h
 * Author: naveen
 *
 * Created on January 3, 2021, 7:46 PM
 */

#ifndef MAX30105_H
#define	MAX30105_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "i2c_simple_master.h"
#include "delay.h"
#include "time_service.h"
# include <string.h>

// OK
#define MAX30105_ADDRESS          0x57 //7-bit I2C Address
#define I2C_SPEED_STANDARD        100000
#define I2C_SPEED_FAST            400000
#define STORAGE_SIZE 4 //Each long is 4 bytes so limit this to fit on your micro
#define I2C_BUFFER_LENGTH 32

bool MAX30105_begin();
uint32_t MAX30105_getRed(void); //Returns immediate red value
uint32_t MAX30105_getIR(void); //Returns immediate IR value
uint32_t MAX30105_getGreen(void); //Returns immediate green value
bool MAX30105_safeCheck(uint8_t maxTimeToCheck); //Given a max amount of time, check for new data
void MAX30105_bitMask(uint8_t reg, uint8_t mask, uint8_t thing);

// Configuration
void MAX30105_softReset();
void MAX30105_setLEDMode(uint8_t mode);
void MAX30105_setADCRange(uint8_t adcRange);
void MAX30105_setSampleRate(uint8_t sampleRate);
void MAX30105_setPulseWidth(uint8_t pulseWidth);
void MAX30105_setPulseAmplitudeRed(uint8_t value);
void MAX30105_setPulseAmplitudeIR(uint8_t value);
void MAX30105_setPulseAmplitudeGreen(uint8_t value);
void MAX30105_setPulseAmplitudeProximity(uint8_t value);

//FIFO Configuration 
void MAX30105_setFIFOAverage(uint8_t samples);
void MAX30105_enableFIFORollover();
void MAX30105_disableFIFORollover();
void MAX30105_setFIFOAlmostFull(uint8_t samples);
  
//FIFO Reading
uint16_t MAX30105_check(void); //Checks for new data and fills FIFO
uint8_t MAX30105_available(void); //Tells caller how many new samples are available (head - tail)
void MAX30105_nextSample(void); //Advances the tail of the sense array
uint32_t MAX30105_getFIFORed(void); //Returns the FIFO sample pointed to by tail
uint32_t MAX30105_getFIFOIR(void); //Returns the FIFO sample pointed to by tail
uint32_t MAX30105_getFIFOGreen(void); //Returns the FIFO sample pointed to by tail

uint8_t MAX30105_getWritePointer(void);
uint8_t MAX30105_getReadPointer(void);
void MAX30105_clearFIFO(void); //Sets the read/write pointers to zero

// Detecting ID/Revision
uint8_t MAX30105_getRevisionID();
uint8_t MAX30105_readPartID();  

// Setup the IC with user selectable settings
void MAX30105_setup(uint8_t powerLevel, uint8_t sampleAverage, uint8_t ledMode, int sampleRate, int pulseWidth, int adcRange);

//activeLEDs is the number of channels turned on, and can be 1 to 3. 2 is common for Red+IR.
uint8_t  MAX30105_activeLEDs; //Gets set during setup. Allows check() to calculate how many bytes to read from FIFO

uint8_t MAX30105_revisionID; 

void MAX30105_readRevisionID();

typedef struct MAX30105_Record
{
    uint32_t red[STORAGE_SIZE];
    uint32_t IR[STORAGE_SIZE];
    uint32_t green[STORAGE_SIZE];
    uint8_t head;
    uint8_t tail;
} MAX30105_sense_struct; //This is our circular buffer of readings from the sensor

MAX30105_sense_struct MAX30105_sense;



#ifdef	__cplusplus
}
#endif

#endif	/* MAX30105_H */


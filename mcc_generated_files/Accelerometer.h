/* 
 * File:   Accelerometer.h
 * Author: naveen
 *
 * Created on January 8, 2021, 7:17 PM
 */

#ifndef ACCELEROMETER_H
#define	ACCELEROMETER_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdio.h>
#include "../ICM_20948_C.h"
#include  "delay.h"
#include "i2c_simple_master.h"

#define I2C_ADDR ICM_20948_I2C_ADDR_AD1

ICM_20948_Status_e my_write_i2c(uint8_t reg, uint8_t* data, uint32_t len, void* user);
ICM_20948_Status_e my_read_i2c(uint8_t reg, uint8_t* buff, uint32_t len, void* user);
void accel_setup();
void accel_read();
void printPaddedInt16b(int16_t val);
void printRawAGMT(ICM_20948_AGMT_t agmt);
float getAccMG(int16_t raw, uint8_t fss);
float getGyrDPS(int16_t raw, uint8_t fss);
float getMagUT(int16_t raw);
float getTmpC(int16_t raw);


#ifdef	__cplusplus
}
#endif

#endif	/* ACCELEROMETER_H */


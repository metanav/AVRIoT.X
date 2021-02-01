/* 
 * File:   i2c_scan.h
 * Author: naveen
 *
 * Created on January 8, 2021, 6:03 PM
 */

#ifndef I2C_SCAN_H
#define	I2C_SCAN_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include <stdint.h>
#include <stdio.h>
#include "../include/twi0_master.h"

    
uint8_t I2C_check(uint8_t  deviceAddress);
void I2C_scan();

#ifdef	__cplusplus
}
#endif

#endif	/* I2C_SCAN_H */


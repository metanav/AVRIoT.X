/*
    MMA7760.h
    Library for accelerometer_MMA7760

    Copyright (c) 2013 seeed technology inc.
    Author        :   FrankieChu
    Create Time   :   Jan 2013
    Change Log    :
    Modified By   : Naveen Kumar
 
    The MIT License (MIT)

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
 */

#include <stdbool.h>
#include "i2c_simple_master.h"
#include "MMA7660.h"

//#define MMA7660TIMEOUT  500         // us

/*Function: Write a byte to the register of the MMA7660*/
void MMA7660_write(uint8_t _register, uint8_t _data) {
    i2c_write1ByteRegister(MMA7660_ADDR, _register, _data);
}

/*Function: Read a byte from the register of the MMA7660*/
uint8_t MMA7660_read(uint8_t _register) {
    return i2c_read1ByteRegister(MMA7660_ADDR, _register);
}


void MMA7660_init() {
    MMA7660_setMode(MMA7660_STAND_BY);
    MMA7660_setSampleRate(AUTO_SLEEP_64);
    MMA7660_setMode(MMA7660_ACTIVE);
}

void MMA7660_setMode(uint8_t mode) {
    MMA7660_write(MMA7660_MODE, mode);
}

void MMA7660_setSampleRate(uint8_t rate) {
    MMA7660_write(MMA7660_SR, rate);
}

/*Function: Get the contents of the registers in the MMA7660*/

/*          so as to calculate the acceleration.            */
bool MMA7660_getXYZ(int8_t* x, int8_t* y, int8_t* z) {
    while (1) {
        uint8_t data[1] = {64};
        i2c_readNBytes(MMA7660_ADDR, data, 1);
        if (data[0] < 64) {
            i2c_readNBytes(MMA7660_ADDR, data, 1);
            i2c_readNBytes(MMA7660_ADDR, data, 1);
            break;
        }
    }
    
    uint8_t val[3];
    i2c_readNBytes(MMA7660_ADDR, val, 3);

    *x = ((int8_t) (val[0] << 2)) / 4;
    *y = ((int8_t) (val[1] << 2)) / 4;
    *z = ((int8_t) (val[2] << 2)) / 4;

    return true;
}
//
//bool MMA7660_getAcceleration(float* ax, float* ay, float* az) {
//    int8_t x, y, z;
//    if (!MMA7660_getXYZ(&x, &y, &z)) {
//        return false;
//    }
//    *ax = x / 21.00;
//    *ay = y / 21.00;
//    *az = z / 21.00;
//
//    return true;
//}

bool MMA7660_getAccXYZ(int8_t *accX, int8_t *accY, int8_t *accZ) {
    int8_t x, y, z;
    
    if (!MMA7660_getXYZ(&x, &y, &z)) {
        return false;
    }
    
    *accX = (x / 21.0) * 10;
    *accY = (y / 21.0) * 10;
    *accZ = (z / 21.0) * 10;
    
    return true;
}



//bool MMA7660_getAccXYZ(float* ax, float* ay, float* az) {
//    int8_t x, y, z;
//    if (!MMA7660_getXYZ(&x, &y, &z)) {
//        return false;
//    }
//    *ax = (x / 21.00) * 9.81f;
//    *ay = (y / 21.00) * 9.81f;
//    *az = (z / 21.00) * 9.81f;
//
//    return true;
//}



#include "i2c_scan.h"

static  twi0_operations_t  NackAddressHandler(void *Ptr)
{    /* Do something to notice that Address Nack have occurred. */
	uint8_t * nackFlag = Ptr;
    *nackFlag = 1;
    return I2C0_STOP;          /* Let driver proceed with Stop signal. */
}

uint8_t I2C_check( uint8_t  deviceAddress)
{
	static uint8_t   NackFlag; 
	static uint8_t   dummy[2] = {0x00, 0x18};
    NackFlag = 0;        /* Reset the flag, */

    while(!I2C0_Open(deviceAddress)); // sit here until we get the bus..

    I2C0_SetBuffer(&dummy[1],2);		    /* This should work with i2c_master.c with no change. */
//  I2C_SetBuffer(&dummy[1],0);			/* Zero length transfer require modified driver i2c_master.c */
	I2C0_SetAddressNackCallback(NackAddressHandler, &NackFlag); /* Pointer to Flag as payload pointer. */

    I2C0_MasterOperation(0);				/* Start a I2C Write operation, same as I2C_MasterWrite. */
    while(I2C0_BUSY ==  I2C0_Close()); // sit here until finished.
    return NackFlag;
}

void I2C_scan()
{	uint8_t address, flag;
	printf("I2C 7bit Address \r\n");
//	for (address = 0x50; address < 0x58; address ++)		/* I2C EEPROM addresses. */
	for (address = 8; address < 120; address ++)			/* Permissible 7-bit addresses. */
	{			//	LATCbits.LATC3 = 1; /* Diagnostic timing signal. */
		flag = I2C_check(address);
				//	LATCbits.LATC3 = 0; /* Diagnostic timing signal. */
		if (flag) {
			//printf(" 0x%X No answer. \r\n", address);
		} else {
			printf(" Found 0x%X. \r\n", address);
        }
	}
}



#ifndef _i2c_H
#define _i2c_H

#include <xc.h>

#define _XTAL_FREQ 64000000 //note intrinsic _delay function is 62.5ns at 64,000,000Hz  
#define _I2C_CLOCK 100000 //100kHz for I2C

//Function prototypes (see .c file for descriptors)
void I2C_2_Master_Init(void);
void I2C_2_Master_Idle(void);
void I2C_2_Master_Start(void);
void I2C_2_Master_RepStart(void);
void I2C_2_Master_Stop(void);
void I2C_2_Master_Write(unsigned char data_byte);
unsigned char I2C_2_Master_Read(unsigned char ack);


#endif

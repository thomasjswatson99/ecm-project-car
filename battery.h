#ifndef _battery_H
#define _battery_H

#include <xc.h>

#define _XTAL_FREQ 64000000 //note intrinsic _delay function is 62.5ns at 64,000,000Hz

//function prototypes
void ADC_init(void);
unsigned int ADC_getval(void);

#endif
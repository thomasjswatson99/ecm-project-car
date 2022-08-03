#ifndef _color_H
#define _color_H

#include <xc.h>
#include "dc_motor.h"

#define _XTAL_FREQ 64000000 //note intrinsic _delay function is 62.5ns at 64,000,000Hz

//definition of RGB structure
struct RGBC_val { 
	signed int R;
	signed int G;
	signed int B;
    signed int C;
};


//Function prototypes (see .c file for descriptors)
void initRGB(void);
void toggleRGB(void);
void toggleR(void);
void toggleG(void);
void toggleB(void);
void color_click_init(void);
void color_writetoaddr(char address, char value);
signed int color_read(char start);
unsigned int color_read_status(void);
void color_read_RGBC(struct RGBC_val *RGBC);
void calibrate(struct RGBC_val *v,struct RGBC_val *n);
void scan(struct RGBC_val *C,struct RGBC_val *R,struct RGBC_val *G,struct RGBC_val *B);
char identifier(struct RGBC_val *aC, struct RGBC_val *aR, struct RGBC_val *aG, struct RGBC_val *aB, struct DC_motor *mL, struct DC_motor *mR);



#endif

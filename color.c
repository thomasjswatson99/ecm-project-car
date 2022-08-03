#include <xc.h>
#include "color.h"
#include "dc_motor.h"
#include "i2c.h"
#include "serial.h"
#include "buggyFunc.h"

#include <stdio.h>

//declare structures
//struct DC_motor motorL, motorR; 		//declare two DC_motor structures
//struct turnDelay turnL, turnR;          //Declare two turnDelay structs

//variable included in function
//char color_number = 0;


/*****************************************************************************
 * Initialise the pins for the colour clicker
 ******************************************************************************/
void initRGB(void){
    LATGbits.LATG0 = 0;     //Initialise Red LED
    TRISGbits.TRISG0 = 0;
    
    LATEbits.LATE7 = 0;
    TRISEbits.TRISE7 = 0;   //Initialise Green LED
    
    LATAbits.LATA3 = 0;
    TRISAbits.TRISA3 = 0;   //Initialise Blue LED
}

/******************************************************************************
 * Turn on the 3 LEDs, using the funcitons which follow
 ******************************************************************************/
void toggleRGB(void)
{
    LATGbits.LATG0 = !LATGbits.LATG0;   //R    
    LATEbits.LATE7 = !LATEbits.LATE7;   //G
    LATAbits.LATA3 = !LATAbits.LATA3;   //B
}

void toggleR(void)
{
    LATGbits.LATG0 = !LATGbits.LATG0;
}

void toggleG(void)
{
    LATEbits.LATE7 = !LATEbits.LATE7; 
}

void toggleB(void)
{
    LATAbits.LATA3 = !LATAbits.LATA3;
}


/******************************************************************************
 * Initialise the colour clicker
 ******************************************************************************/
void color_click_init(void)
{   
    //setup colour sensor via i2c interface
    I2C_2_Master_Init();      //Initialise i2c Master

    color_writetoaddr(0x00, 0x01);     //set device PON
    __delay_ms(3);                      //need to wait 3ms for everthing to start up
    
    color_writetoaddr(0x00, 0x03);      //turn on device ADC
    color_writetoaddr(0x01, 0xD5);      //set integration time to 42 cycles
}


/******************************************************************************
 * Send a byte to the specified address over i2c
 ******************************************************************************/
void color_writetoaddr(char address, char value){
    I2C_2_Master_Start();         //Start condition
    I2C_2_Master_Write(0x52 | 0x00);     //7 bit device address + Write mode
    I2C_2_Master_Write(0x80 | address);    //command + register address
    I2C_2_Master_Write(value);    
    I2C_2_Master_Stop();          //Stop condition
}


/******************************************************************************
 * Used for debugging, to inform if the interrupt has triggered
 ******************************************************************************/
//unsigned int color_read_status(void)
//{
//	unsigned int tmp;
//	I2C_2_Master_Start();         //Start condition
//	I2C_2_Master_Write(0x52 | 0x00);     //7 bit address + Write mode
//	I2C_2_Master_Write(0xA0 | 0x13);    //command (Repeated byte protocol transaction) + start at status
//	I2C_2_Master_RepStart();			// start a repeated transmission
//	I2C_2_Master_Write(0x52 | 0x01);     //7 bit address + Read (1) mode
//	tmp=I2C_2_Master_Read(0);			//read the status byte
//	I2C_2_Master_Stop();          //Stop condition
//	return tmp;
//}

/******************************************************************************
 * Read any prescribed ADC value
 ******************************************************************************/
signed int color_read(char start)
{
    /*
     * For reference:
     * Red : 0x16
     * Green : 0x18
     * Blue : 0x1A
     * Clear : 0x14
     */
    signed int tmp;
	I2C_2_Master_Start();         //Start condition
	I2C_2_Master_Write(0x52 | 0x00);     //7 bit address + Write mode
	I2C_2_Master_Write(0xA0 | start);    //command (auto-increment protocol transaction) + start at CLEAR low register
	I2C_2_Master_RepStart();			// start a repeated transmission
	I2C_2_Master_Write(0x52 | 0x01);     //7 bit address + Read (1) mode
	tmp=I2C_2_Master_Read(1);			//read the Red LSB
	tmp=tmp | (I2C_2_Master_Read(0)<<8); //read the Red MSB (don't acknowledge as this is the last read)
	I2C_2_Master_Stop();          //Stop condition
	return tmp;
}

/******************************************************************************
 * Return all ADC values to a struct
 ******************************************************************************/
void color_read_RGBC(struct RGBC_val *RGBC)
{
    RGBC->R = color_read(0x16);
    RGBC->G = color_read(0x18);
    RGBC->B = color_read(0x1A);
    RGBC->C = color_read(0x14);
}


/******************************************************************************
 * Read all the ADC values, when different lights are turned on
 ******************************************************************************/
void scan(struct RGBC_val *C,struct RGBC_val *R,struct RGBC_val *G,struct RGBC_val *B)
{
    __delay_ms(200);
    color_read_RGBC(C);
    
    //Values when only red is on
    toggleG();
    toggleB();
    __delay_ms(200);
    color_read_RGBC(R);
    
    //when only green is on
    toggleG();
    toggleR();
    __delay_ms(200);
    color_read_RGBC(G);
    
    //when only blue is on
    toggleG();
    toggleB();
    __delay_ms(200);
    color_read_RGBC(B);
    toggleG();
    toggleR(); 
}


/******************************************************************************
 * Remove cross talk
 ******************************************************************************/
void calibrate(struct RGBC_val *v, struct RGBC_val *n){
    v->R = v->R - n->R;
    v->G = v->G - n->G;
    v->B = v->B - n->B;
    v->C = v->C - n->C;
}

/******************************************************************************
 * This long function returns the colour of the card
 ******************************************************************************/
char identifier(struct RGBC_val *aC,struct RGBC_val *aR,struct RGBC_val *aG,struct RGBC_val *aB, struct DC_motor *mL, struct DC_motor *mR)
{

    struct RGBC_val vals, redVals, greenVals, blueVals;
    unsigned char color_number = 0;
    
    scan(&vals, &redVals, &greenVals, &blueVals);   //'scan' across different colours
//    
    calibrate(&vals, aC);       //Remove cross talk & reflections from the ground etc
    calibrate(&redVals,aR);
    calibrate(&greenVals,aG);
    calibrate(&blueVals,aB);
    
    unsigned int tracker = 0;   //This variable will be used for determining colour
/******************************************************************************
 * These tests function as follows:
 * 
 * - first do the comparisons to expected values
 * - Assign each test a specific bit in the 'tracker' variable, The test result 
 *   may be high or low
 * 
 * - perform the bitwise and operator, to dictate whether the test is critical 
 *   or not, as some test results vary
 * - Compare this new 'and operated' int to the expected for each colour, and
 *   assign colour number
 * 
 ******************************************************************************/    
    if (vals.C > 150){tracker+=1;}
    if (blueVals.B > 30){tracker+=(1<<1);}
    
    if ((tracker&0b11) == 0b01){color_number = 1;}      //These two colour numbers stop
    else if ((tracker&0b11) == 0b11){color_number = 3;} //The closest
    
    else {
        shuffle(mL, mR, 5, 1);                          //Move forwards for !black and !dblue
        scan(&vals, &redVals, &greenVals, &blueVals);   //Scan again
        
        if (blueVals.B>90){tracker+=(1<<2);}
        if (greenVals.G>600){tracker+=(1<<3);}
        if (greenVals.G>320){tracker+=(1<<4);}
        if (redVals.R>550){tracker+=(1<<5);}
        if (vals.R>730){tracker+=(1<<6);}
        if (vals.G>245){tracker+=(1<<7);}
        if (blueVals.B>60){tracker+=(1<<8);}
        if (blueVals.B>50){tracker+=(1<<9);}
        if (vals.C<950){tracker+=(1<<10);}
        
        //   tracker& critical?     ==   result int
        if ((tracker&0b00101001101) == 0b00100000000){color_number = 2;}
        if ((tracker&0b11111111100) == 0b11110000100){color_number = 3;}
        if ((tracker&0b10111111101) == 0b00110010100){color_number = 4;}
        if ((tracker&0b10101011101) == 0b10000000000){color_number = 5;}
        if ((tracker&0b10111101101) == 0b00010000000){color_number = 6;}
        if ((tracker&0b00111001101) == 0b00000000000){color_number = 7;}
        if ((tracker&0b00101101101) == 0b00000100000){color_number = 8;}
        if ((tracker&0b01111101101) == 0b01001100000){color_number = 9;}
        if ((tracker&0b01011101101) == 0b01011100000){color_number = 10;}
    }
    
    
    /*
     * This section for sending information over serial during debugging
     */
    
//    char serialR[40];
//    char *strr = &serialR;
//    char serialG[40];
//    char *strg = &serialG;
//    char serialB[40];
//    char *strb = &serialB;
//    char serialC[40];
//    char *strc = &serialC;
//    
//    sendCharSerial4(10);
//    sendCharSerial4(13);
//    
//    sprintf(serialR,"%d, %d, %d, %d", redVals.R, redVals.G, redVals.B, redVals.C);
//    sendStringSerial4(strr);
//    __delay_ms(10);
//    sendCharSerial4(10);
//    sendCharSerial4(13);
//    __delay_ms(10);
//    
//    sprintf(serialG,"%d, %d, %d, %d", greenVals.R, greenVals.G, greenVals.B, greenVals.C);
//    sendStringSerial4(strg);
//    __delay_ms(10);
//    sendCharSerial4(10);
//    sendCharSerial4(13);
//    __delay_ms(10);
//    
//    sprintf(serialB,"%d, %d, %d, %d", blueVals.R, blueVals.G, blueVals.B, blueVals.C);
//    sendStringSerial4(strb);
//    __delay_ms(10);
//    sendCharSerial4(10);
//    sendCharSerial4(13);
//    __delay_ms(10);
//    
//    sprintf(serialC,"%d, %d, %d, %d", vals.R, vals.G, vals.B, vals.C);
//    sendStringSerial4(strc);
//    __delay_ms(10);
//    sendCharSerial4(10);
    
    /***************************************************************************
    * Serial section over
     **************************************************************************/
 
    /***************************************************************************
     * Section below may be uncommented for debugging: returns number of card found
     **************************************************************************/    
//    char debugCount = color_number;
//    
//    while (debugCount>0){   //This just flashes for the colour number
//        togRTurn();         //Used for debugging
//        __delay_ms(300);
//        togRTurn();
//        __delay_ms(300);
//        debugCount--;
//    }
   
    return color_number;
    
}

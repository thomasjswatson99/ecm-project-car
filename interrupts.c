#include <xc.h>
#include <pic18f67k40.h>
#include "interrupts.h"
#include "i2c.h"
#include "color.h"
#include "dc_motor.h"
#include "buggyFunc.h"

/*****************************************************************************
 * This function initialises the Interrupts on the PIC, using the pin connected
 * to the interrupt on the color clicker
 ******************************************************************************/
void Interrupts_init(void)
{
    //initialise interrupt pin for colour sensing
    TRISBbits.TRISB0 = 1;       //Set B0 as input
    ANSELBbits.ANSELB0 = 0;     //Turn off analogue input
    INT0PPSbits.PORT = 0b001;   //Set up the correct port for int0
    INT0PPSbits.PIN = 0;        //Use pin B0 for int0
    INTCONbits.INT0EDG = 0;     //Interrupt on falling edge of INT0 pin
   
    PIE0bits.TMR0IE = 1;        //Enable timer interrupter
    
    INTCONbits.GIEL = 1;        //Enables all unmasked peripheral interrupts
    PIE0bits.INT0IE = 1;        //declare interrupt
    INTCONbits.GIE = 1;         //Enables all global interrupts
}


/******************************************************************************
 * This function initialises the colour clicker's interrupt, over i2c
 ******************************************************************************/
void Interrupts_colorclick_init(void)
{
    color_writetoaddr(0x04,0x00);           //set low byte of low reg to 0
    color_writetoaddr(0x05,0x00);           //set high byte of low reg to 0
    
    
    unsigned int threshold = 890;           //Input desired interrupt
    char threshLow = (char)(threshold&0b11111111);  //Here for easier debugging upt threshold
    char threshHigh = (char)((threshold-threshLow)>>8); 
    color_writetoaddr(0x06,threshLow);          //set low byte of high reg
    color_writetoaddr(0x07,threshHigh);         //Set high byte of high reg     
    

                                     
    color_writetoaddr(0x0C,0b1);                //1 clear channel value out of range to trigger
    color_writetoaddr(0x00,0b00011011);         //initialise interrupt
    
    //start I2C 
    I2C_2_Master_Start();
    I2C_2_Master_RepStart();
}


/******************************************************************************
 * High priority interrupt service routine
 * Make sure all enabled interrupts are checked and flags cleared
******************************************************************************/
void __interrupt(high_priority) HighISR()
{
	//if interrupt triggered, toggle LED D7
    //colour read in main when triggered 
    if (PIR0bits.INT0IF){
        LATDbits.LATD7 = 1;
//        PIE0bits.INT0IE = 0; 
        PIR0bits.INT0IF = 0;                //clear interrupt 
    } 
    
    if (PIR0bits.TMR0IF){                   //if timer interrupt triggered
        LATHbits.LATH3 = 1;   //toggle LED
        PIR0bits.TMR0IF = 0;                //reset flag
    }
}

/******************************************************************************
 * Clear the interrupt on the colour clicker; such that it may trigger another
 ******************************************************************************/
void color_interrupt_clearReset(void)
{
    I2C_2_Master_Start();                //Start condition
    I2C_2_Master_Write(0x52 | 0x00);     //7 bit address (of the slave peripheral) + Write mode
    I2C_2_Master_Write(0b11100110);      //clears interrupt
    I2C_2_Master_Stop();
}


/*************************************
 * Function to set up timer 0
************************************/
void Timer0_init(void)
{
    T0CON1bits.T0CS=0b010;      // Fosc/4 
    T0CON1bits.T0ASYNC=1;       //   input to the TMR0 counter is synchronized to Fosc/4
    T0CON1bits.T0CKPS=0b0110;   // 1:64  (prescalar) (closest value to 1/4s given equation)
    T0CON0bits.T016BIT=1;       //16bit mode	
	
    // it's a good idea to initialise the timer registers so we know we are at 0
    TMR0H=3035 >> 8;            //start the timer at 3035
    TMR0L=3035 & 255;          //to get as close to 1/4 second as possible (from T=4*PS/Fosc)
    T0CON0bits.T0EN=1;          //start the timer
    
}


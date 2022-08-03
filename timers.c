#include <xc.h>
#include "timers.h"

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

/************************************
 * Function to return the full 16bit timer value
 * Note TMR0L and TMR0H must be read in the correct order, or TMR0H will not contain the correct value
************************************/
unsigned int get16bitTMR0val(void)
{
	//add your code to get the full 16 bit timer value here
    
    unsigned int tmr = 0;       //set up new variable for total time
    tmr = 256*TMR0H + TMR0L;    //combine TMROL and TMROH to get total count
    tmr = tmr>>8;               //bit shift total value to display 8 most significant bits
                
    return tmr;                 //return 8 most significant bits

}


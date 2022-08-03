
#include <xc.h>
#include "buggyFunc.h"


/*****************************************************************************
 * Initialise all the pins which will be used for indicating the status of 
 * the buggy
 ******************************************************************************/
void initDebugs(void){
    //LED initialised for debugging
    LATDbits.LATD7 = 0;         //Set initial value
    TRISDbits.TRISD7 = 0;       //Set RD7 as output
    
    LATHbits.LATH3 = 0;         //Set initial value
    TRISHbits.TRISH3 = 0;
    
    //initialise start button
    TRISFbits.TRISF2 = 1;   
    ANSELFbits.ANSELF2=0; //turn off analogue input on pin 
    
    //initialise Headlamps (RH1)
    LATHbits.LATH1 = 0;
    TRISHbits.TRISH1 = 0;
    
    //Initialise Mainbeams (RD3)
    LATDbits.LATD3 = 0;
    TRISDbits.TRISD3 = 0;
    
    //Initialise Break (RD4)
    LATDbits.LATD4 = 0;
    TRISDbits.TRISD4 = 0;
    
    //Initialise Left indicator (RF0)
    LATFbits.LATF0 = 0;
    TRISFbits.TRISF0 = 0;
    
    //Initialise Right indicator (RH0)
    LATHbits.LATH0 = 0;
    TRISHbits.TRISH0 = 0;
}


void togHLamp(void){
    LATHbits.LATH1 = !LATHbits.LATH1;
}

void togMBeam(void){
    LATDbits.LATD3 = !LATDbits.LATD3;
}

void togBrake(void){
    LATDbits.LATD4 = !LATDbits.LATD4;
}

void togLTurn(void){
    LATFbits.LATF0 = !LATFbits.LATF0;
}

void togRTurn(void){
    LATHbits.LATH0 = !LATHbits.LATH0;
}

/*****************************************************************************
 * Flashes an SOS signal if buggy gets 'lost'
 ******************************************************************************/

void SOS(void){
    
    char a = 0;         //set up variable a
    while(a<6)          //send 3 short bursts
    {
    togHLamp();
    __delay_ms(100);
    a++;                
    }
    while(a<13)         //send 3 long bursts
    {
    togHLamp();
    __delay_ms(1000);
    a++;
    }
    while(a<20)        //send 3 short bursts
    {
    togHLamp();
    __delay_ms(100);
    a++;
    }
}


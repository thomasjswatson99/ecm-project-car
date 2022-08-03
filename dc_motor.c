#include "dc_motor.h"
#include "buggyFunc.h"
#include <xc.h>


/*****************************************************************************
 * This function initialises the timer2 for use for the PWM control, and also
 * The PWM module
 ******************************************************************************/
void initDCmotorsPWM(unsigned char PWMperiod, struct DC_motor *mL, struct DC_motor *mR){
	//initialise your TRIS and LAT registers for PWM
    LATEbits.LATE0 = 0;
    LATCbits.LATC7 = 0;
    TRISEbits.TRISE2 = 0;
    TRISCbits.TRISC7 = 0;
    TRISEbits.TRISE4 = 0;
    TRISGbits.TRISG6 = 0;

    // timer 2 config
    T2CONbits.CKPS=0b11;        // 1:8 prescaler
    T2HLTbits.MODE=0b00000;     // Free Running Mode, software gate only
    T2CLKCONbits.CS=0b0001;     // Fosc/4

    // Tpwm*(Fosc/4)/prescaler - 1 = PTPER
    T2PR=PWMperiod;             //Period reg 10kHz base period
    T2CONbits.ON=1;
    
    RE2PPS=0x0A;                //PWM6 on RE2
    RC7PPS=0x0B;                //PMW7 on RC7

    PWM6DCH=0;                  //0% power
    PWM7DCH=0;                  //0% power
    
    PWM6CONbits.EN = 1;         //Enable the PWMs
    PWM7CONbits.EN = 1;
    
    //set up motorL
    mL->power=0; 						//zero power to start
    mL->direction=0; 					//set default motor direction
    mL->dutyHighByte=(unsigned char *)(&PWM6DCH);	//store address of PWM duty high byte
    mL->dir_LAT=(unsigned char *)(&LATE);           //store address of LAT
    mL->dir_pin=4; 						//pin RE4 controls direction
    mL->PWMperiod=PWMcycle; 			//store PWMperiod for motor
    
    //same for motorR but different PWM register, LAT and direction pin
    mR->power=0; 						//zero power to start
    mR->direction=0; 					//set default motor direction
    mR->dutyHighByte=(unsigned char *)(&PWM7DCH);	//store address of PWM duty high byte
    mR->dir_LAT=(unsigned char *)(&LATG);           //store address of LAT
    mR->dir_pin=6; 						//pin RG6 controls direction
    mR->PWMperiod=PWMcycle; 			//store PWMperiod for motor
    
    //send the commands to the PWM module
    setMotorPWM(mL);
    setMotorPWM(mR);
}


/******************************************************************************
 * Function to set PWM output from the values in the motor structure
 ******************************************************************************/
void setMotorPWM(struct DC_motor *m)
{
	int PWMduty; //tmp variable to store PWM duty cycle

	if (m->direction){  //if forward
                        // low time increases with power
		PWMduty=m->PWMperiod - ((int)(m->power)*(m->PWMperiod))/100;
	}
	else {              //if reverse
                        // high time increases with power
		PWMduty=((int)(m->power)*(m->PWMperiod))/100;
	}

	*(m->dutyHighByte) = (char)PWMduty; //set high duty cycle byte
        
	if (m->direction){ // if direction is high
		*(m->dir_LAT) = (unsigned char)(*(m->dir_LAT) | (1<<(m->dir_pin))); // set dir_pin bit in LAT to high without changing other bits
	} else {
		*(m->dir_LAT) = (unsigned char)(*(m->dir_LAT) & (~(1<<(m->dir_pin)))); // set dir_pin bit in LAT to low without changing other bits
	}
}


/******************************************************************************
 * Function to initialise turn delays
 * All turns performed in one motion rather than in bursts 
 * so any turn errors aren't magnified
 * Delays calibrated for motor and surface used in testing
 * Reduced performance at low battery calibrated for
 ******************************************************************************/

void setTurnDelay(struct turnDelay *tL, struct turnDelay *tR, unsigned int battery){
   
    if (battery==100){         //if running at normal power assign delays
        tL->thirty =0;
        tL->sixty = 15;
        tL->ninety = 80;
        tL->oneThirtyFive = 255;
        tL->oneEighty = 395; 
        
        tR->thirty = 0;
        tR->sixty = 5;
        tR->ninety = 75;
        tR->oneThirtyFive = 220;  
    }
    else{                           //reduced performance found at low power
                                    //longer delay needed
                                    //delay increases proportionally as battery decreases to zero
        
        tL->thirty = (20-battery)/2;
        tL->sixty = 15 + 3*(20-battery);
        tL->ninety = 80 + (3*(20-battery));
        tL->oneThirtyFive = 255 + 3*(20-battery);
        tL->oneEighty = 395 + 4*(20 - battery);
        
        tR->thirty = (20-battery)/2;
        tR->sixty = 5 + 3*(20 - battery);
        tR->ninety = 75 + 3*(20-battery);
        tR->oneThirtyFive = 220 + 3*(20-battery);
    }
    tL->direction = 0;              //directions assigned
    tR->direction = 1;
}
      
/******************************************************************************
 * Function to stop the buggy motors gradually
 ******************************************************************************/
void stop(struct DC_motor *mL, struct DC_motor *mR)
{
    while (mL->power>0){        //while power is not 0
        mL->power--;            //decreasing power
        mR->power--;
        setMotorPWM(mL);        //feedback to PWM function every increment
        setMotorPWM(mR);
        __delay_ms(5);          //Delay so it isn't instant
    }   
}

/******************************************************************************
 * Function to make the robot turn based on direction and delay
 *****************************************************************************/

void turnAngle(struct DC_motor *mL, struct DC_motor *mR, unsigned int delay, unsigned int direction)
{
    //First setup for right turns
    if (direction==1){             //if direction is right 
        togRTurn();
        mL->direction=1;            
        mR->direction=0;
    }
    else{                         //if direction is left
        togLTurn();
        mL->direction=0;            
        mR->direction=1;
    }
    while (mL->power<60){        //increase power up to 60
        mL->power++;
        mR->power++;
        setMotorPWM(mL);
        setMotorPWM(mR);
        __delay_ms(5);
    }
    
    //Set delay to change angle
    delay /= 5;
    unsigned int i = 0;
    while (i<delay){
        __delay_ms(5);  
        i++;
    }
    
    //reduce power back to zero
    while (mL->power>0){
        mL->power--;
        mR->power--;
        setMotorPWM(mL);
        setMotorPWM(mR);
        __delay_ms(5);
    }          
    
    //switch turn lights back off
    if (LATHbits.LATH0 == 1){
        togRTurn();   
    }   
    if (LATFbits.LATF0 ==1){
        togLTurn(); 
    }  
}

/******************************************************************************
 * Function sends the buggy moving forwards or backwards
 ******************************************************************************/
void fullSpeedAhead(struct DC_motor *mL, struct DC_motor *mR, unsigned int nomPower, char forward)
{
    mL->direction=forward;            //Set the buggy to travel forwards         
    mR->direction=forward;
    
    //gradually increase to desired power-----------------------------------------
    
    while (mL->power<nomPower){
        mL->power++;            //Increase the power in each by 1
        mR->power++;            
        setMotorPWM(mL);        //Update the PWM every increment
        setMotorPWM(mR);        
        __delay_ms(5);          //Delay a little so speed ramps up gently
        } 
}


/******************************************************************************
 * Function returns the associated delay and direction, depending on card colour
 ******************************************************************************/
struct turnInfo turnFunction(struct turnDelay *tL, struct turnDelay *tR, char C){
    
    struct turnInfo turnSpecs;
    
    if (C == 2){        //purple
        turnSpecs.direction = tR->direction;
        turnSpecs.delay = tR->oneThirtyFive;
    }
    else if (C == 3){   //dark blue
        turnSpecs.direction = tL->direction;
        turnSpecs.delay = tL->sixty;
    }
    else if (C == 4){    //light blue
        turnSpecs.direction = tL->direction;
        turnSpecs.delay = tL->oneEighty;
    }
    else if (C == 5){   //dark green
        turnSpecs.direction = tL->direction;
        turnSpecs.delay = tL->ninety;
    }
    else if (C == 6){   //light green
        turnSpecs.direction = tL->direction;
        turnSpecs.delay = tL->oneThirtyFive;
    }
    else if (C == 7){   //yellow
        turnSpecs.direction = tR->direction;
        turnSpecs.delay = tR->thirty;
    }
    else if (C == 8){   //orange
        turnSpecs.direction = tL->direction;
        turnSpecs.delay = tL->thirty;
    }
    else if (C == 9){   //red
        turnSpecs.direction = tR->direction;
        turnSpecs.delay = tR->ninety;
    }
    else if (C == 10){   //pink
        turnSpecs.direction = tR->direction;
        turnSpecs.delay = tR->sixty;
    }
    return turnSpecs;  
}

/******************************************************************************
 * Function returns the opposite card colour for the same angle turn, 
 * corresponding to a different direction
 ******************************************************************************/
char returnSwitch(char C)
{
    if(C==2){return 6;}
    if(C==3){return 10;}
    if(C==4){return 4;}
    if(C==5){return 9;}
    if(C==6){return 2;}
    if(C==7){return 8;}
    if(C==8){return 7;}
    else{return 5;}
}


/******************************************************************************
 * Function rolls the buggy a little bit, for small adjustments
 ******************************************************************************/
void shuffle(struct DC_motor *mL, struct DC_motor *mR, char dist, char dir)
{
    mL->direction=dir;          //Set the buggy to travel in intended direction        
    mR->direction=dir;          //Don't need to update until any changes to power
    
    unsigned int pwr = 50 - 40 * dir;
    
    while (mL->power<(pwr))
    {
        mL->power++;            //Increase the power in each by 1
        mR->power++;            
        setMotorPWM(mL);        //Update the PWM every increment
        setMotorPWM(mR);        
        __delay_ms(5);          //Delay a little so speed ramps up gently
    }
    while (dist>0){
        __delay_ms(200);
        dist--;
    }
    while (mL->power>0){
        mL->power--;            //Increase the power in each by 1
        mR->power--;            
        setMotorPWM(mL);        //Update the PWM every increment
        setMotorPWM(mR);        
        __delay_ms(5);          //Delay a little so speed ramps up gently
    }
}



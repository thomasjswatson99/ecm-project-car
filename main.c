// CONFIG1L
#pragma config FEXTOSC = HS     // External Oscillator mode Selection bits (HS (crystal oscillator) above 8 MHz; PFM set to high power)
#pragma config RSTOSC = EXTOSC_4PLL// Power-up default value for COSC bits (EXTOSC with 4x PLL, with EXTOSC operating per FEXTOSC bits)

// CONFIG3L
#pragma config WDTCPS = WDTCPS_31// WDT Period Select bits (Divider ratio 1:65536; software control of WDTPS)
#pragma config WDTE = OFF        // WDT operating mode (WDT enabled regardless of sleep)

#include "serial.h"
#include "i2c.h"
#include "dc_motor.h"
#include "color.h"
#include "interrupts.h" 
#include "buggyFunc.h"
#include "battery.h"
#include <xc.h>
#include <stdio.h>

void main(void){
    
//INITIALISATION FUNCTIONS--------------------------------------------------
    initUSART4();                   //serial initialisation
    color_click_init();             //color click initialisation
    Interrupts_colorclick_init();   //color click interrupt initilisation
    Interrupts_init();              //timer0 and external0 interrupt initialisation
    initDebugs();                   //debug feature
    initRGB();                      //RGB initialisation
    ADC_init();                     //battery file initialisation
    Timer0_init();                  //timer file initialisation
    

//DECLARE VARIABLES-------------------------------------------------------------

    char battery_level[10];             //String used for sending battery level over serial
    unsigned int level;                 //Battery level
    unsigned int batterySaver = 100;    //set default batterySaver to 100 (indicates buggy works normally)
    
    unsigned int forwardPower = 26;     //power variables, for rolling speeds
    unsigned int reversePower = 40;
    
    char color;                         //char for the colour of card found
    char store[10];                     //Record of cards found
    char time[10];                      //Record of distances between cards
    signed int motions = 0;             //Number of card+travels
    char complete = 0;                  //Is the course complete?
    char interval = 0;                  //Counts distance travelled in 0.25s
    char maxTime = 23;                  //Time intervals until 2m travelled
    
    //DECALRE STRUCTURES--------------------------------------------------------
    struct DC_motor motorL, motorR; 		//declare two DC_motor structures
    struct turnDelay turnL, turnR;          //Declare two turnDelay structs for turning right and left
    struct RGBC_val ambientR, ambientG, ambientB, ambientC; //Ambient conditions scanned without card; eliminates cross talk

    //SET UP BUTTONS------------------------------------------------------------
    TRISFbits.TRISF2=1;     //set TRIS value for pin (input)
    ANSELFbits.ANSELF2=0;   //turn off analogue input on pin 

    //BATTERY LEVEL-------------------------------------------------------------
    level = 20* ADC_getval()/21;        //adjust battery reading to get a percentage out of 100 
    sprintf(battery_level,"battery level %d%",level);   //send battery level to serial
    sendStringSerial4(battery_level);   //In this config, battery level is sent over serial if connected
    
    /****************************************************************************
     * Battery saver mode:
     * Accommodates for less power provided by motor at lower battery
     * Buggy worked as expected until it reached a level of 20
     * Under a battery level of 20, the calibrated delays were no longer accurate
    ******************************************************************************/
    
    if (level<20)
    {
        batterySaver = level;   //change power utilised in turning based on battery level to ensure correct angle of turn
        togMBeam();
        __delay_ms(200);    //Quick red flash to warn battery is low
        togMBeam();
    }
    if(level>90)
    {  
        togMBeam();
        __delay_ms(200);    //Quick flash to indicated battery is high
        togMBeam();
    }
    
    
    setTurnDelay(&turnL, &turnR, batterySaver); //Assigns the turn times, based on calibration and battery
    
    while (PORTFbits.RF2);                          //Wait for button press
    initDCmotorsPWM(200, &motorL, &motorR);         //Initialise the PWM for motors
    toggleRGB();                                    //Turn on the front RGB LEDs
    scan(&ambientC,&ambientR,&ambientG,&ambientB);  //Scan the conditions using an ambient scan
    color_interrupt_clearReset();                   
    
    while (PORTFbits.RF2);
    //START BUGGY JOURNEY-----------------------------------------------------------

    while((motions < 10)&&(complete == 0))  //while less than 10 cards have been found
    {  
       
        fullSpeedAhead(&motorL,&motorR,forwardPower,1);     //drive buggy forwards

        if (LATHbits.LATH3==1)  //Timer interrupt triggers this LED
        {      
            interval++;         //increment time every time timer interrupt
            LATHbits.LATH3=0;   //Occurs (timer 0)
        }

        
        /***************************************************************
         * CARD FOUND
         ***************************************************************/
      
        if (LATDbits.LATD7)                     //Interrupt triggers this LED
        {   
            togBrake();                         //toggle brake
            stop(&motorL,&motorR);              //stop buggy
            togBrake();
            color = identifier(&ambientC,&ambientR,&ambientG,&ambientB, &motorL, &motorR);
            store[motions] = color;             //store the colour located
                      
            
            if (color == 1)                     //if black card found, return home
            {
                LATDbits.LATD7 = 0;             //switch off colour click interrupt LED
                color_interrupt_clearReset();   //clear interrupt in i2c
                complete = 1;                   //leave while loop to return home
                time[motions] = interval;       //Store time to reach
                store[motions] = 0;             //Store 0 in the turn record
            }

            else                                    //if any other coloured card found
            {
                shuffle(&motorL, &motorR, 1, 0);    //Buggy retreats from the card a bit
                time[motions] = interval;           //Store arbitrary time to reach card
                motions ++;                         //add one to keep track of how many cards found
                
                //sendStringSerial4(time[motions]);   //send time to serial (only required for debug)

                struct turnInfo turnSpecs = turnFunction(&turnL, &turnR, color);    //define a struct for turn delays from color
                turnAngle(&motorL, &motorR, turnSpecs.delay, turnSpecs.direction);  //send to the turn angle function
                interval = 0;                       //reset time passed to 0
                LATDbits.LATD7 = 0;                 //switch off colour click interrupt LED
                color_interrupt_clearReset();       //clear interrupt in i2c    
            }     
        }
        
        /**********************************************************************
         * NO CARD FOUND         
         *********************************************************************/
           
        if (interval > maxTime)             //if travelled more than 2m, buggy is lost
        {                  
            time[motions] = interval;       //Store arbitrary time travelled 
            store[motions] = 0;             //no motion performed
            complete = 1;                   //leave while loop and return home
            togBrake();
            stop(&motorL,&motorR);
            togBrake();
            SOS();                          //Flash the SOS lights!
        }
    }  
   
    //TELL US HOW MANY CARDS FOUND--- (DEBUG FEATURE) --------------------------------
    unsigned int count = 0;
    while (count<motions)
    {
        LATFbits.LATF0=1;
        __delay_ms(1000);
        LATFbits.LATF0=0;
        count++;
        __delay_ms(100);
    }
    //DEBUG FEATURE ABOVE OVER -------------------------------------------------
 
    //RETURN JOURNEY-----------------------------------------------------------
    
    interval=0;                                     //reset time to 0
    toggleRGB();                                    //turn off LEDs

    while(motions>=0)               //loop until all actions complete
    {        
        if (store[motions]!=0)
        {
            store[motions] = returnSwitch(store[motions]);                  //Switch the angle of the turns
            struct turnInfo oppTurn;                                        //store in structure
            oppTurn = turnFunction(&turnL, &turnR, store[motions]);         //Aquire turn data
            turnAngle(&motorL, &motorR, oppTurn.delay, oppTurn.direction);  //perform relevant turn  
        }
        
        fullSpeedAhead(&motorL,&motorR,reversePower,0);                     //set buggy running backwards
        
        while(time[motions]>0)                                              //runs until all motions complete
        {
            if (LATHbits.LATH3==1)          //if interrupt triggers
            {
                if (time[motions]%2 == 0)   //HLamp toggled ever 2nd timer interrupt
                {
                    togHLamp();                    
                }
                time[motions]--;            //Decrease the counter
                LATHbits.LATH3=0;           //Turn off interrupt LED
            }
        }
        stop(&motorL,&motorR);              //stop buggy
        motions--;                          //Decrease the counter for which motion is occuring
    }       
}



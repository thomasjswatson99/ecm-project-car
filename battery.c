#include <xc.h>
#include "battery.h"

/************************************
* ADC_initialisation
* to sample on pin RE1
************************************/
void ADC_init(void)
{
    TRISFbits.TRISF6=1; // Select pin RE1 as input
    ANSELFbits.ANSELF6=1; //Ensure analogue circuitry is active 

    ADREFbits.ADNREF = 0;       // Use Vss (0V) as negative reference
    ADREFbits.ADPREF = 0b00;    // Use Vdd (3.3V) as positive reference
    ADPCH=0b101110 ;            // Select channel RF6/ANF61 for ADC
    ADCON0bits.ADFM = 0;        // Left-justified result (i.e. no leading 0s)
    ADCON0bits.ADCS = 1;        // Use internal Fast RC (FRC) oscillator as clock source for conversion
    ADCON0bits.ADON = 1;        // Enable ADC
}


/******************************************************************************
 * Return the ADC value
 ******************************************************************************/
unsigned int ADC_getval(void)
{
    unsigned int tmpval;
    
    ADCON0bits.GO = 1;      // Start ADC conversion
    while (ADCON0bits.GO);  // Wait until conversion done (bit is cleared automatically when done)        
    tmpval = ADRESH;        // 8 most significant bits of the ADC result 
    return tmpval;          //return this value when the function is called
}

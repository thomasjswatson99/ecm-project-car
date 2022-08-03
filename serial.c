#include <xc.h>
#include "serial.h"

void initUSART4(void) {

	//code to set up USART4 for Reception and Transmission 
    
    RC0PPS = 0x12; // Map EUSART4 TX to RC0
    RX4PPS = 0x11; // RX is RC1
    
    BAUD4CONbits.BRG16 = 0; 	//set baud rate scaling
    TX4STAbits.BRGH = 0; 		//high baud rate select bit
    SP4BRGL = 51;              //set baud rate to 51 = 19200bps
    SP4BRGH = 0;                //not used

    RC4STAbits.CREN = 1; 		//enable continuous reception
    TX4STAbits.TXEN = 1;        //enable transmitter
    RC4STAbits.SPEN = 1; 		//enable serial port   
}


//function to check the TX reg is free and send a byte
void sendCharSerial4(char charToSend) {
    while (!PIR4bits.TX4IF); // wait for flag to be set
    //LATHbits.LATH3 = !LATHbits.LATH3;  //to see when input is taken
    TX4REG = charToSend; //transfer char to transmitter
}


//function to send a string over the serial interface
void sendStringSerial4(char *string){
	while(*string != 0){  // While the data pointed to isn't a 0x00 do below (strings in C end with NULL byte) 
		sendCharSerial4(*string++); 	//Send out the current byte pointed to and increment the pointer
    }
}

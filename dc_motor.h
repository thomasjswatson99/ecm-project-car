#ifndef _DC_MOTOR_H
#define _DC_MOTOR_H

#include <xc.h>

#define _XTAL_FREQ 64000000
#define PWMcycle 199        //PWM cycle

struct DC_motor {       //definition of DC_motor structure
    char power;         //motor power, out of 100
    char direction;     //motor direction, forward(1), reverse(0)
    unsigned char *dutyHighByte; //PWM duty high byte address
    unsigned char *dir_LAT; //LAT for dir pin
    char dir_pin;       // pin number that controls direction on LAT
    unsigned char PWMperiod;      //base period of PWM cycle
};

struct turnDelay{               //This struct can hold the delays for all the turn angles
    unsigned int thirty;        //Time delay to turn 30 degrees
    unsigned int sixty;         //'' 60 degrees
    unsigned int ninety;
    unsigned int oneThirtyFive;
    unsigned int oneEighty;
    char direction;             //Set the direction of the turn
};

struct turnInfo{                //information regarding turn
    char direction;
    unsigned int delay;
};

//Function prototypes (see .c file for descriptors)
void initDCmotorsPWM(unsigned char PWMperiod, struct DC_motor *mL, struct DC_motor *mR); // function to setup PWM
void setMotorPWM(struct DC_motor *m);
void setTurnDelay(struct turnDelay *tL, struct turnDelay *tR, unsigned int batterySaver);
void stop(struct DC_motor *mL, struct DC_motor *mR);
void fullSpeedAhead(struct DC_motor *mL, struct DC_motor *mR, unsigned int nomPower, char forward);
void turnAngle(struct DC_motor *mL, struct DC_motor *mR, unsigned int delay, unsigned int direction);
struct turnInfo turnFunction(struct turnDelay *tL, struct turnDelay *tR, char C);
char returnSwitch(char C);
void shuffle(struct DC_motor *mL, struct DC_motor *mR, char dist,  char dir);

#endif

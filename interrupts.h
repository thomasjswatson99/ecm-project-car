#ifndef _interrupts_H
#define _interrupts_H

#define _XTAL_FREQ 64000000

 
//Function prototypes (see .c file for descriptors)
void Interrupts_init(void);
void Interrupts_colorclick_init(void);
void __interrupt(high_priority) HighISR();
void color_interrupt_clearReset(void);
void Timer0_init(void);

#endif
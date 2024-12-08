//
//  time4io.h
//  
//
//  Created by Sreenija Veladri on 2024-02-20.
//
#ifndef time4io_h
#define time4io_h
#include <stdio.h>
#endif /* time4io_h */
#include <stdint.h> 
#include <pic32mx.h> 
#include "mipslab.h"


/* PART f*/
int getsw(void){ // to encode switch 4,3,2,1 (in binary ) to swap 

    int switches = (PORTD >> 8) & 0x0F; // reads value and shifts by 8 and aligns switches(11 to 8) to lsb by masking  
    return switches;
}

/* PART g*/
int getbtns(void){ //to encode button 4,3,2 (in binary ) to swap 


    int buttons = (PORTD >> 5) & 0x07; // reads value and shifts by 5 and aligns buttons(7,6,5) to lsb  by masking 
    return buttons;

}
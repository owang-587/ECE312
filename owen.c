#include "defines.h"
#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "hd44780.h"
#include "lcd.h"

//ext osc. 20MHz


int main(void){

    //timer for 1ms overflow
    


    //set up PWM when alarm = 1

    //initialize output pin, set low
    PORTB &= ~(1<<PB1);
    DDRB |= (1<<PB1); 
    
    //initialize PWM, N=8
    ICR1 = 1250; //TOP value
    OCR1A = 625; //75% duty cycle at equilibrium
    TCCR1A = (1<<COM1B1)|(1<<WGM11); //generate waveform in normal mode
    TCCR1B = (1<<WGM13)|(1<<CS11); //(WGM13:10 = 0b1010), (CS12:10 = 0b010) 

    return (0);
}
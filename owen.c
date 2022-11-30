#include "defines.h"
#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "hd44780.h"
#include "lcd.h"

//ext osc. 20MHz //CKDIV8


int main(void){

    //CTC mode
    

    //set up PWM when alarm = 1    
    //initialize PWM, N=8
    ICR1 = 1250; //TOP value
    OCR1A = 625; //50% duty cycle at equilibrium
    TCCR1A = (1<<COM1B1)|(1<<WGM11); //generate waveform in normal mode
    TCCR1B = (1<<WGM13)|(1<<CS10); //(WGM13:10 = 0b1010), (CS12:10 = 0b001) 

    return (0);
}

/*
freq calc
f_PWM = f_clk/(2*N*TOP)
f_PWM = 1ms = 1000Hz
f_clk = 20MHz/8 = 2.5MHz
N*TOP = 1250 ------> N = 1
*\
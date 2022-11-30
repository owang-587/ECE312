/* 
 * File:   main.c
 * Author: parker
 *
 * Created on November 29, 2022, 8:22 PM
 */

#include "defines.h"
#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "hd44780.h"
#include "lcd.h"

static volatile int MIN, HOUR, ALM_MIN,ALM_HR, ALRM;

ISR(INT0_vect){
    //disable INT1
    EIMSK &= ~(1<<INT1);
    
    //Reset current set time
    MIN = 0;
    HOUR = 0;
    uint8_t mode_select = 0; //0 for Clock time, 1 for Alarm time
    uint8_t time_select = 0; //0 for Minutes, 1 for Hours
    
    //wait for SW3(PD4) to be pressed
    while(!(PORTD&(1<<PD4))){
        if(!(PORTD&(1<<PD2))){
            mode_select ^= 1; //toggle mode
            _delay_ms(50);
        }
        if(!(PORTD&(1<<PD6))){
            time_select ^= 1; //toggle mode
            _delay_ms(50);
        }
        switch(mode_select){
            case 0: //Clock Time
                if(time_select==0){
                    if(MIN<60){
                        MIN++;
                    }
                    if(MIN==60){ //if the minutes is at 60 increment hour
                        if(HOUR<24){
                            HOUR++;
                        }
                        MIN = 0;
                    }
                }
                else{
                    if(HOUR<24){
                        HOUR++;
                    }
                    if(HOUR==24){
                        HOUR = 0;
                    }
                }
            case 1: //Alarm Time
                if(time_select==0){
                    if(ALM_MIN<60){
                        ALM_MIN++;
                    }
                    if(ALM_MIN==60){
                        if(ALM_HR<24){ //if the minutes is at 60 increment hour
                            ALM_HR++;
                        }
                        ALM_MIN = 0;
                    }
                }
                else{
                    if(ALM_HR<24){
                        ALM_HR++;
                    }
                    if(ALM_HR==24){
                        ALM_HR = 0;
                    }
                }
                
        }
        
        //if both alarm minutes and alarm hours are not zero, alarm is set
        if (((ALM_MIN&&ALM_HR)!=0)&&mode_select==1){
            ALRM = 1;
        }
    }

    //Renable INT1
    EIMSK |= (1<<INT1);
}

ISR(INT1_vect){
    //Reset alarm block
}

void main_init(){
    //PD1 = SW5 -> INT0, begin set time routine 
    //PD2 = SW4 -> INT1, reset alarm, toggle set between alarm and clock
    //PD4 = SW3 -> exit set time routine
    //PD5 = SW2 -> used in set time, increments time
    //PD6 = SW1 -> toggles between minutes and hours in set time
    
    //PD6:2 as inputs for buttons
    DDRD &= ~((1<<PD6)|(1<<PD5)|(1<<PD4)|(1<<PD3)|(1<<PD2));
    //Enable pull up on PORTD for buttons
    PORTB |= (1<<PD6)|(1<<PD5)|(1<<PD4)|(1<<PD3)|(1<<PD2);
    //Enable PB0(BUZZER),PB3(LED) as output
    DDRB |= (1<<PB0)|(1<<PB3);
    //Start pins low
    PORTB = 0;
    //Enable INT0,INT1 for low level
    EICRA =0;
    //Enable INT0,INT1
    EIMSK |= (1<<INT0)|(1<<INT1); 
}

int main(void){
    //Initialize pins and enable external interrupts
    main_init();
    
    sei();
    while(1){
        //Turn on LED if alarm is set
        if(ALRM==1){
            PORTB |= (1<<PB3);
        }
        else{
            PORTB &= ~(1<<PB3);
        }

        //Condition for alarm to go off
        if((ALRM==1)&&((ALM_MIN&&ALM_HR)==0)){
            PORTB |= (1<<PB0)|(1<<PB3);
            _delay_ms(50);
            PORTB &= ~((1<<PB0)|(1<<PB3));
            _delay_ms(50);
        }
    }
    
    return(0);
}
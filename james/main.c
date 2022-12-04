/* 
 * File:   main.c
 * Author: james
 *
 * Created on November 29, 2022, 5:47 PM
 */

#define F_CPU 1000000UL

#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <string.h>
#include <util/delay.h>
#include "C:\Users\james\MPLABXProjects\ECE312_FinalProjectSegment.X\defines.h"
#include "C:\Users\james\MPLABXProjects\ECE312_FinalProjectSegment.X\lcd.h"
#include "C:\Users\james\MPLABXProjects\ECE312_FinalProjectSegment.X\hd44780.h"
/*
 * 
 */

/*
    Hours. Minutes correspond to the clock
    almHours, almMinutes correspond to the alarm
    timeState is used to configure either the hours or minutes of both the clock and alarm
    alarmState is used to determine if the alarm has been set or not
*/
volatile uint8_t Hours, Minutes;
volatile uint8_t almHours, almMinutes;
volatile int timeState = 0;
volatile int configState = 0;
volatile int alarmState = 0;

void ioInit();
void exintInit();

ISR(INT0_vect, ISR_BLOCK) {
    /* PD6 (SW1) -- Toggle between Hours and Minutes
       PD5 (SW2) -- Increment the selected value
       PD4 (SW3) -- Confirm selection and exit ISR
       PD3 (SW4) -- Toggle between setting the Alarm and setting the Clock 
       configState == 0 -- Select Alarm
       configState == 1 -- Select Clock
       timeState == 0 -- Select Hours
       timeState == 1 -- Select Minutes
       alarmState == 0 -- Alarm is not set
       alarmState == 1 -- Alarm is set*/
    EIFR |= (1<<INTF0); // Clear the interrupt flag
    EIMSK &= ~(1<<INT1); // Disable external interrupt 1
    
    // Reset values
    almHours = 0;
    almMinutes = 0;
    configState = 0;
    timeState = 0;
    alarmState = 0;
    
    // Wait for PD4 button to be pressed to exit the while loop
    while(!(PIND & (1<<PD4))) {
        // If PD6 button is pressed, toggle between selecting the hours or minutes
        if (!(PIND & (1<<PD6))) {
            timeState ^= 1; // Toggle configState
        }
        
        // If PD5 button is pressed and the alarm is selected for changing, increment selected time value
        if (!(PIND & (1<<PD5)) && configState == 0) {
            if (timeState == 0 && almHours < 23) { // Increment hours from 0 to 23 hours
                almHours++;
            } else if (timeState == 0 && almHours == 23) { // When incrementing at 23, reset to 0 hours which is equivalent to 24 hours
                almHours = 0;
            } else if (timeState == 1 && almMinutes < 59) { // Increment minutes from 0 to 59 minutes
                almMinutes++;
            } else if (timeState == 1 && almMinutes == 59) { // When incrementing at 59 minutes, reset to 0 minutes which is equivalent to 60 minutes
                almMinutes = 0;
            }
        }

        // If PD5 button is pressed and the clock is selected for changing, increment selected time value
        if (!(PIND & (1<<PD5)) && configState == 1) {
            if (timeState == 0 && almHours < 23) { // Increment hours from 0 to 23 hours
                almHours++;
            } else if (timeState == 0 && almHours == 23) { // When incrementing at 23, reset to 0 hours which is equivalent to 24 hours
                almHours = 0;
            } else if (timeState == 1 && almMinutes < 59) { // Increment minutes from 0 to 59 minutes
                almMinutes++;
            } else if (timeState == 1 && almMinutes == 59) { // When incrementing at 59 minutes, reset to 0 minutes which is equivalent to 60 minutes
                almMinutes = 0;
            }
        }
        
        // If PD3 button is pressed, toggle between setting the alarm or the clock
        if (!(PIND & (1<<PD3))) {
            configState ^= 1;
        }
    }
    
    // Set the alarmState to 'on' if an alarm value was set
    if (configState == 0 && (almHours > 0 || almMinutes > 0)) {
        alarmState = 1;
    }    
    
    EIMSK |= (1<<INT1); // Enable external interrupt 1
}


int main(void) {
    ioInit(); // Initialize I/O, namely buttons
    exintInit(); // Initialize external interrupt 0
    
    sei();
    while(1) {
        if(alarmState == 1) {
            PORTB |= (1<<PB3); // Turn LED on
        } else {
            PORTB &= ~(1<<PB3); // Turn LED off
        }
    }
    
    return (EXIT_SUCCESS);
}

void ioInit(){
    DDRD &= ~((1<<PD2)|(1<<PD3)|(1<<PD4)|(1<<PD5)|(1<<PD6)); // Enable PD2, PD3, PD4, PD5, PD6 as input
    PORTD |= (1<<PD2)|(1<<PD3)|(1<<PD4)|(1<<PD5)|(1<<PD6); // Enable pull-up on PD2, PD3, PD4, PD5, PD6
    
    DDRB |= (1<<PB3); // Make PB3 an output
    PORTB &= ~(1<<PB3); // Output low
}

void exintInit(){
    EICRA &= ~((1<<ISC01)|(1<<ISC00)); // Low level of INT0 generates an interrupt request
    EIMSK |= (1<<INT0); // Enable external interrupt 0
}
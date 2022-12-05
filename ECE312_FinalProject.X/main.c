/* 
 * File:   main.c
 * Author: james
 *
 * Created on December 4, 2022, 3:22 PM
 */

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
#include "defines.h"
#include "lcd.h"
#include "hd44780.h"
/*
 * 
 */

FUSES = {
	.low = 0x7F, // LOW {SUT_CKSEL=EXTXOSC_8MHZ_XX_16KCK_14CK_65MS, CKOUT=CLEAR, CKDIV8=SET}
	.high = 0xD9, // HIGH {BOOTRST=CLEAR, BOOTSZ=2048W_3800, EESAVE=CLEAR, WDTON=CLEAR, SPIEN=SET, DWEN=CLEAR, RSTDISBL=CLEAR}
	.extended = 0xFF, // EXTENDED {BODLEVEL=DISABLED}
};

LOCKBITS = 0xFF; // {LB=NO_LOCK, BLB0=NO_LOCK, BLB1=NO_LOCK}

/*
    Hours. Minutes correspond to the clock
    almHours, almMinutes correspond to the alarm
    timeState is used to configure either the hours or minutes of both the clock and alarm
    alarmState is used to determine if the alarm has been set or not
*/
volatile uint8_t Hours, Minutes, Seconds;
volatile uint8_t almHours, almMinutes, almSeconds;
volatile int timeState = 0;
volatile int configState = 0;
volatile int alarmState = 0;

void mcu_Init();

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
            if (timeState == 0 && Hours < 23) { // Increment hours from 0 to 23 hours
                Hours++;
            } else if (timeState == 0 && Hours == 23) { // When incrementing at 23, reset to 0 hours which is equivalent to 24 hours
                Hours = 0;
            } else if (timeState == 1 && Minutes < 59) { // Increment minutes from 0 to 59 minutes
                Minutes++;
            } else if (timeState == 1 && Minutes == 59) { // When incrementing at 59 minutes, reset to 0 minutes which is equivalent to 60 minutes
                Minutes = 0;
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

ISR(INT1_vect){
    alarmState = 0; // Setting the alarm countdown off
    almHours = 99; // No alarm time
    
    OCR0A = 0; //0% duty cycle at equilibrium
    
    PORTB &= ~(1<<PB0); //Setting the Piezo buzzer to '0'
    PORTB &= ~(1<<PB3); // Toggling the LED to off   
} 

FILE lcd_str = FDEV_SETUP_STREAM ( lcd_putchar , NULL , _FDEV_SETUP_WRITE); // to create global variable for LCD stream

int main(void) {
    mcu_Init(); // Initialize registers
    int counter = 0;
    int almCounter = 0;
    lcd_init();
    
    sei();
    
    while(1){
        while (!(TIFR1 &(1<<OCF1A)));
        TIFR1 |= (1<<OCF1A); //clear flags
        
        cli();
        
       /* This chain of if statements is to determine when to change the time on the clock*/
        counter++; // counter is in ms
        if (counter == 1000) {
            Seconds++; // counter = 1000 means that 1 seconds has passed
            counter = 0;
        }
        if (Seconds == 60) {
            Minutes++; // seconds = 60 means 1 minute has passed
            Seconds = 0;
        }
        if (Minutes == 60) {
            Hours++; // minutes = 60 means 1 hour has passed
            Minutes = 0;
        }
        if (Hours == 24) {
            Hours = 0; // hours = 24 means that we are back to 0
        }
        
        /* This chain of if statements is for counting down the alarm*/
        if (alarmState == 1) {
            almCounter++; // almCounter is in ms
        }
        if(almCounter == 1000 && alarmState == 1){
            if (almSeconds <= 59 && almSeconds > 0){
                almSeconds--;
            } else if (almSeconds == 0 && almMinutes > 0){
                almMinutes--;
                almSeconds = 59;
            }
            almCounter = 0;
        }
        if (almSeconds == 0 && alarmState == 1){
            if (almMinutes > 0) {
                almMinutes--;
                almSeconds = 59;
            }
            if (almMinutes <= 59 && almMinutes > 0){
                almMinutes--;
            }
        }
        if (almMinutes == 0 && alarmState == 1){
            if (almHours > 0) {
                almHours--;
                almMinutes = 59;
            }
            if (almHours <= 60 && almHours > 0) {
                almHours--;
            }
        }  
        if (almHours == 0 && almMinutes == 0 && almSeconds == 0 && alarmState == 1){
                // Trigger alarm
                OCR0A = 78; //50% duty cycle at equilibrium
            }
        if (alarmState == 0) {
            almHours = 99;
        }
        sei();
        
        char clkBuffer[10] = "\0";
        char almBuffer[10] = "\0";
        
        sprintf(clkBuffer, "%02i:%02i", Hours, Minutes);
        fprintf(&lcd_str, "\x1b\x01Clock   %s", clkBuffer);
        
        sprintf(almBuffer, "%02i:%02i:%02i", almHours, almMinutes, almSeconds);
        fprintf(&lcd_str, "\x1b\x40Alarm   %s", almBuffer); 
        
        if(alarmState == 1) {
            PORTB |= (1<<PB3); // Turn LED on
        } else {
            PORTB &= ~(1<<PB3); // Turn LED off
        }
    }
    
    return (EXIT_SUCCESS);
}

void mcu_Init(){
    DDRD &= ~((1<<PD2)|(1<<PD3)|(1<<PD4)|(1<<PD5)|(1<<PD7)); // Enable PD2, PD3, PD4, PD5, PD7 as input
    PORTD |= (1<<PD2)|(1<<PD3)|(1<<PD4)|(1<<PD5)|(1<<PD7); // Enable pull-up on PD2, PD3, PD4, PD5, PD7
    
    DDRD |= (1<<PD6); // Initialize OC0A as an output
    PORTD &= ~(1<<PD6); // Output low on PD6
    
    DDRB |= (1<<PB3); // Make PB3 an output
    PORTB &= ~(1<<PB3); // Output low
    
    //CTC mode (timer 1)
    TCCR1A = 0;
    TCCR1B |= (1<<WGM12) | (1<<CS10); //mode 4, pre-scaler = 1
    OCR1A = (2500-1); //interrupt every 1 ms 
    TIFR1 |= (1<<OCF1A); //clear existing flags
    
    // External Interrupt initialization
    EICRA &= ~((1<<ISC01)|(1<<ISC00)); // Low level of INT0 generates an interrupt request
    EIMSK |= (1<<INT0)|(1<<INT1); // Enable external interrupt 0
       
    //initialize PWM, N=8
    TCCR0A = (1<<WGM00) | (1<<COM0A1); //set OC0A on compare-match when down-count
    TCCR0B = (1<<WGM02) | (1<<CS01); //mode 5, pre-scaler = 8
}



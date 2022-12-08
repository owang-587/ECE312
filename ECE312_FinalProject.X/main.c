/* 
 * File:   main.c
 * Author: james, parker, owen, wasi
 *
 * Created on November 29, 2022, 5:47 PM
 */
#include "defines.h"
#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <string.h>
#include <util/delay.h>
#include "lcd.h"
#include "hd44780.h"


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
    almState is used to determine if the alarm has been set or not
*/
volatile uint8_t Hours, Minutes, Seconds = 0;
volatile uint8_t almHours, almMinutes, almSeconds = 0;
volatile int timeState, cfgState, almState = 0;
volatile int setMode = 0;
volatile uint8_t prevSec = 0;
volatile int counter, almCounter = 0;

void mcu_Init();
void clkIncrement();
void printClkTime();
void printAlmTime();
void clkMode();
void timeChange();

ISR(INT0_vect, ISR_BLOCK) {
    _delay_ms(50);
    setMode ^= 1;
    _delay_ms(50);
}

ISR(INT1_vect){
    almState = 0; // Setting the alarm countdown off
    almHours = 99; // No alarm time
    almMinutes = 0; 
    
    OCR0A = 0; //0% duty cycle at equilibrium
    PORTB &= ~(1<<PB0); // Toggling the LED to off   
} 

FILE lcd_str = FDEV_SETUP_STREAM ( lcd_putchar , NULL , _FDEV_SETUP_WRITE); // to create global variable for LCD stream

int main(void) {
    mcu_Init(); // Initialize registers
    sei();
    
    lcd_init(); // Initialize LCD 16X2
    fprintf(&lcd_str, "\x1b\x0c"); // Turn cursor off

    while(1){
        if (setMode == 0) {
            clkMode(); // Default mode to display the current clock time and alarm time
            if (almHours == 0 && almMinutes == 0 && almSeconds == 0 && almState == 1){
                // Trigger alarm
                OCR0A = 39;} //50% duty cycle at equilibrium
        } else {
            timeChange(); // Mode to change either the clock or alarm
        }
    }
    
    return (EXIT_SUCCESS);
}

void mcu_Init(){
    DDRD &= ~((1<<PD2)|(1<<PD3)|(1<<PD4)|(1<<PD5)|(1<<PD7)); // Enable PD2, PD3, PD4, PD5, PD7 as input
    PORTD |= (1<<PD2)|(1<<PD3)|(1<<PD4)|(1<<PD5)|(1<<PD7); // Enable pull-up on PD2, PD3, PD4, PD5, PD7
    
    DDRD |= (1<<PD6); // Initialize OC0A as an output
    PORTD &= ~(1<<PD6); // Output low on PD6
    
    DDRB |= (1<<PB0); // Make PB0 an output
    PORTB &= ~(1<<PB0); // Output low
    
    //CTC mode (timer 1)
    TCCR1A = 0;
    TCCR1B |= (1<<WGM12) | (1<<CS10); //mode 4, pre-scaler = 1
    OCR1A = (2500-1); //interrupt every 1 ms 
    TIFR1 |= (1<<OCF1A); //clear existing flags
    
    // External Interrupt initialization
    EICRA &= ~((1<<ISC01)|(1<<ISC00)); // Low level of INT0 generates an interrupt request
    EIMSK |= (1<<INT0)|(1<<INT1); // Enable external interrupt 0
       
    //initialize PWM, N=8
    TCCR0A = (1<<WGM00) | (1<<COM0A1); //MODE 1, set OC0A on compare-match when down-count
    TCCR0B = (1<<CS01); //pre-scaler = 8
    
    TCCR2A = 0;
}

void clkIncrement(){
       /* This chain of if statements is to determine when to change the time on the clock*/
        counter++; // counter is in ms
        if (counter == 1000 && almState == 0) {
            Seconds++; // counter = 1000 means that 1 seconds has passed
            counter = 0;
        }
        if (counter == 1000 && almState == 1) {
            Seconds++; // counter = 1000 means that 1 seconds has passed
            if (almSeconds <= 59 && almSeconds > 0){
                almSeconds--;
            }
            if (almSeconds == 0 && almMinutes != 59){
                if (almMinutes < 59 && almMinutes > 0){
                    almMinutes--;
                    almSeconds = 59;
                }
            }
            if (almSeconds == 0 && almMinutes == 59){
                if (almMinutes > 0){
                    almSeconds = 59;
                }
            }
            if (almMinutes == 0){
                if (almHours <= 23 && almHours > 0) {
                    almHours--;
                    almMinutes = 59;
                }
            } 
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
}

void printClkTime(){
    char clkBuffer[20];
    sprintf(clkBuffer, "%02u:%02u:%02u", Hours, Minutes, Seconds);
    fprintf(&lcd_str, "Clock   %s", clkBuffer);
}

void printAlmTime(){ 
    char almBuffer[20];      
    sprintf(almBuffer, "%02u:%02u:%02u", almHours, almMinutes, almSeconds);
    fprintf(&lcd_str, "Alarm   %s", almBuffer); 
}

void clkMode(){
    while (!(TIFR1 &(1<<OCF1A)));
    TIFR1 |= (1<<OCF1A); //clear flags
    
    // Refresh LCD every second for the displayed clock time
    if (prevSec != Seconds) {
        if (Seconds == 60) {
            cli();
            prevSec = 0;
            sei();
        }  
        
        fprintf(&lcd_str, "\x1b\x01");
        printClkTime();
        fprintf(&lcd_str, "\x1b\xc0");
        printAlmTime();
        if (almHours == 0 &&  almMinutes == 0 && almSeconds == 0 && almState == 1) {
            fprintf(&lcd_str, "\x1b\xc0Reset Alarm     ");
        } else if (almHours == 0 &&  almMinutes == 0 && almSeconds == 0 && almState == 0) {
            fprintf(&lcd_str, "\x1b\xc0Set Alarm       ");
        } else if (almHours == 99 && almState == 0) {
            fprintf(&lcd_str, "\x1b\xc0Set Alarm       ");
        }
    }

    cli();
    
    prevSec = Seconds;
    clkIncrement();

    if (almState == 0) {
        almHours = 99;
    }
    sei();
        
    // If the alarm is set, indicate that it is using the LED
    if(almState == 1) {
        PORTB |= (1<<PB0); // Turn LED on
    } else {
        PORTB &= ~(1<<PB0); // Turn LED off
    }
}

void timeChange(){
    /* PD7 (SW1) -- Toggle between Hours and Minutes
    PD5 (SW2) -- Increment the selected value
    PD4 (SW3) -- Toggle between setting the Alarm and setting the Clock 
    configState == 0 -- Select Alarm
    configState == 1 -- Select Clock
    timeState == 0 -- Select Hours
    timeState == 1 -- Select Minutes
    almState == 0 -- Alarm is not set
    almState == 1 -- Alarm is set*/
    if (cfgState == 1) {
        Seconds = 0; // Reset seconds when setting the clock
    }
    if (cfgState == 0) {
        almSeconds = 0; // Reset alarm seconds when setting the alarm
    }
    
    if(!(PIND & (1<<PD7))) {
        _delay_ms(50); // Button debouncing
        timeState ^= 1; // Toggle the time state
    }
    
    if(!(PIND & (1<<PD4))) {
        _delay_ms(50); // Button debouncing
        cfgState ^= 1;
    }
    
    // If PD5 button is pressed and the alarm is selected for changing, increment selected time value
        if (!(PIND & (1<<PD5)) && cfgState == 0) {
            _delay_ms(50);
            if (timeState == 0 && almHours < 23) { // Increment hours from 0 to 23 hours
                almHours++;
            } else if (timeState == 0 && almHours == 23) { // When incrementing at 23, reset to 0 hours which is equivalent to 24 hours
                almHours = 0;
            } else if (timeState == 0 && almHours > 24) {
                almHours = 0;
            } else if (timeState == 1 && almMinutes < 59) { // Increment minutes from 0 to 59 minutes
                almMinutes++;
            } else if (timeState == 1 && almMinutes == 59) { // When incrementing at 59 minutes, reset to 0 minutes which is equivalent to 60 minutes
                almMinutes = 0;
            }
        }

        // If PD5 button is pressed and the clock is selected for changing, increment selected time value
        if (!(PIND & (1<<PD5)) && cfgState == 1) {
            _delay_ms(50);
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
    
    if (almHours != 99 && (almHours > 0 || almMinutes > 0)) {
        almState = 1;
    }
    
    _delay_ms(200);
    if (cfgState == 0 && timeState == 0) {
        // This if is for setting alarm hours
        fprintf(&lcd_str, "\x1b\x01  Alm Hours: %u", almHours);
    }
    if (cfgState == 0 && timeState == 1) {
        // This if is for setting alarm minutes
        fprintf(&lcd_str, "\x1b\x01 Alm Minutes: %u", almMinutes);
    }
    if (cfgState == 1 && timeState == 0) {
        // This if is for setting clock hours
        fprintf(&lcd_str, "\x1b\x01    Hours: %u", Hours);
    }
    if (cfgState == 1 && timeState == 1) {
        // This if is for setting clock minutes
        fprintf(&lcd_str, "\x1b\x01   Minutes: %u", Minutes);
    }
}
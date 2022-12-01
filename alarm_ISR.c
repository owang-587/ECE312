/* 
 * File:   alarm_ISR.c
 * Author: wasii
 *
 * Created on November 29, 2022, 9:08 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
/*
 * 
 */
ISR(INT1_vect){
    PORTB&=~(1<<PB0); //Setting the Piezo buzzer to '0'
    alarmState = 0; // Setting the alarm countdown off
    PORTB&=~(1<<PB3); // Toggling the LED to off 
    
} 


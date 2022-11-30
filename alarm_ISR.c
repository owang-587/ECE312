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
    PORTB&=~(1<<PB0);
    ALARM = 0;
    PORTB&=~(1<<PB3);
    
} 


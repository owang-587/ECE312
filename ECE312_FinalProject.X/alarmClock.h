/* 
 * File:   alarmClock.h
 * Author: james
 *
 * Created on December 5, 2022, 5:33 PM
 */

#ifndef ALARMCLOCK_H
#define	ALARMCLOCK_H

#ifdef	__cplusplus
extern "C" {
#endif

void mcu_Init();
void clkIncrement();
void almDecrement();
void printClkTime();
void printAlmTime();
void clkMode();
void timeChange();


#ifdef	__cplusplus
}
#endif

#endif	/* ALARMCLOCK_H */


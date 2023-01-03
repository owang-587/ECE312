# UAlberta ECE312 Final Design Project - Alarm Clock

### Basic Functions
Allows users to set and clear both clock and alarm. Both displayed on a 24 hour clock with hour:minute:second (00:00:00) format. 
The set and clear functions are accessed via 5 different buttons. 

### Operating Principle
**Setting the Clock / Arming the Alarm:**
1) Press SW5 to enter the mode to set the clock/alarm. The default setting is to set the alarm first. Note that you can only set either the alarm or the clock separately as the MCU will not remember the values for both.
2) To switch to setting the clock, press SW3.
3) To increment the currently selected value, press SW2. There is no decrement button so you will have to cycle through to the TOP value (60 for minutes and 24 for hours) where the MCU will reset the value back to 0.
4) To toggle between selecting hours or minutes, press SW1. The hours for the clock are labeled “Hours,” the minutes for the clock are labeled “Minutes,” the hours for the alarm are labeled “Alm Hours,” and the minutes for the alarm are labeled “Alm Minutes.”
5) To finish setting your clock or alarm, press SW5. The device will automatically switch back to displaying the selected clock and/or alarm. When an alarm is set, the LED will be on.

**In order to reset the alarm:**
1) The alarm will trigger if the alarm was set and decremented to the value of 00:00:00. An LED will be on and the piezo buzzer will be ringing when the alarm needs to be reset.
2) To reset the alarm, simply press SW4.

### Contributions
James Shin ([jjshin1](https://github.com/jjshin1)), Parker Kung ([parkung02](https://github.com/parkung02)), Wasi Hossein ([wasihsn](https://github.com/wasihsn)), Owen Wang (me)

# ESP32 Study helper /POMODORO TIMER 

A DIY Pomodoro timer built using an ESP32, an OLED display, and a buzzer. It can be controlled from a phone through Bluetooth.

##Parts List
ESP32-WROOM-32,0.96-inch OLED display (I2C),  passive buzzer, breadboard, jumper wires, and USB cable

##What It Does
The timer helps manage work and break sessions using the Pomodoro technique. You control it from your phone using Bluetooth commands, while the OLED displays the timer and session information.

## Features
- Bluetooth control from a phone
- 25- or 45-minute work sessions
- 5- or 15-minute breaks
- OLED timer display
- Buzzer alerts when sessions start and finish
- Tracks completed sessions
- Enforces a work-break cycle

## Possible Future Features
- Custom work and break durations
- Pause and resume
-Save session counts after restarting
-Improved OLED interface
-Daily productivity statistics
  ## What i learned
I learned how to program an ESP32 with C++, communicate with a phone through Bluetooth, and display information on an OLED screen.

One challenge was handling the different timer states, such as waiting, working, and resting, using 'enum' and `switch case`. I also learned to handle Bluetooth commands, fix logic errors, and use 'millis()' to run countdowns without blocking other tasks.

This gave me a better understanding of how to develop program logic and connect software to physical hardware.

# GPS Unit

This github repository details the construction of a GPS unit with custom clock and speed screens.....
![alt text](Images/GPSunit.jpg "GPS unit")

It is based on a GPS receiver board with included antenna using the NEO6MV2 module, outputting NMEA data at 9600 baud each second. All is coordinated by a MEGA 2560 compatible microcontroller board. Information is displayed on a 128x64 LCD screen. Control is effected by just two push buttons interfaced to the microcontroller with a hardware debouncing circuit utilizing a 74HC14 Schmitt trigger IC. Power at 5.1V is assumed to be supplied into a USB-A female connector.

## USAGE

The unit will start up when powered via the USB-A connector at 5.1V.

It might take a long time to get a fix on satelites. Be patient. During that time some data fields will be blank or display NA.

Control of the unit is effected by just two push buttons situated on the left of the LCD screen. We will name them LB and RB (left and right buttons respectively) for later use.

The unit can also be powered and/or programmed from the exposed USB-B port of the Arduino MEGA. If only powered via the Arduino MEGA the LCD display might be dimmer.
Powering the unit simultaniously via the USB-A and the Arduino MEGA USB-B connectors does not seem to cause problems, thought it is probably best avoided.

![alt text](Images/InCase.jpg "GPS unit case")

There are three screens available:

1. Displays the current time, date, lattitude, longtitude, speed (SOG) and height (ASL). A calculated day of the week is shown as (m, t, w, T, f, s, S) and displayed before the date. There is a facility to edit an offset to the UTC for displaying local time. This is retained when power is off. This screen is intended for navigating in the great outdoors.

    <img src="Images/Screen1.jpg" width="300" height="200">

    The UTC offset occupies 4 character positions at the top right of the screen. +11 in the picture above. Non-edit mode is indicated by a "●" appearing under the 4th character. Press RB to enter edit mode, with an underscore appearing under the 1st character to be edited and "●" disappearing. Now press LB several times to set the 1st character (the available characters being "blank",+,-,0,1,2,3,4,5,6,7,8,9 and ½). Press RB again and then RL several times to set the 2nd character. Repeat for 3rd and 4th characters. Finally press RB which exits edit mode and "●" reappears. This error checks the input for validity, reformats it if necessary and redisplays it. An error becomes "  +0" while eg. "-½ 1" becomes "  -½".

1. Just displays the time (UTC+adjustment) and date, with a calculated day of week shown as (Mon, Tue, Wed, Thu, Fri, Sat, Sun). Large fonts are used. This screen is intended for use as a household clock. In the top right had corner of the screen is displayed the adjustment in hours (and/or halves) to UTC that gives us the actual displayed time and date.

    <img src="Images/Screen2.jpg" width="300" height="200">
1. Just displays the speed over ground (SOG) in km/h. It uses a larger font and display only to the nearest km/h, ie. no decimals. This screen is intended as a vehicle speedometer.

    <img src="Images/Screen3.jpg" width="300" height="200">

On power up the 1st screen is displayed. You can cycle through the screens by repeatedly pressing LB.
## HARDWARE

There are just 5 modules in this build. Below we describe them and their interconnections:

1. The Arduino MEGA 2560 microcontroller [module](Images/MEGA2560.jpg)
    - Power is supplied to the POWER 5V and POWER GND pins from J1-1 and J1-2 on the main circuit board.
    - The pushbutton interrupt pins D2 and D3 are connected to J4-1 and J4-2 on the main circuit board.
    - The GPS serial connection is effected by the D19 (RX1) pin being connected to the TX pin on the GPS module. (The RX direction is not implemented).
    - The LCD display is controlled by D53, D51 and D52 being connected to pins 4, 5 and 6 respectively on the LCD module interfacing circuit board. These are labelled RS, R/W and E on the actual LCD board).
1. The GPS Receiver [module](Images/GPSmodule.jpg)
    - Power is supplied to the VCC and GND pins from J2-1 and J2-2 on the main circuit board.
    - The TX pin is connected to the Arduino MEGA as detailed above.
    - The RX pin is unconnected.
1. The LCD module with attached [interfacing circuit board](Images/GPSboardB.png)
    - Power is supplied to the VCC and GND pins (ie. J3-2 and J3-1) from J3-1 and J3-2 on the main circuit board.
    - The display is controlled via pins J5-1, J5-2 and J5-3 connected the to Arduino MEGA as detailed above.
1. The [main circuit board](Images/GPSboardA.png) for power input and hardware debouncing.
    - The veroboard layout is given [here](Images/GPSboardA.pdf), with copper tracks assumed underneath.
    - Powered by the USB-A connector.
    - Power supplied to three other modules (arduino MEGA, GPS Receiver and LCD module) via J1, J2 and J3 as described above.
    - Hardware debouncing to Arduino MEGA via J4 as described above.
    - Connected to pushbutton containing mini circuit board. J5-1, J5-2 (Ground) and J5-3 are connected to J6, J7 and J8 respectively on the mini circuit board
1. The [mini circuit board](Images/miniboard.jpg) that contains the two pushbuttons used for interfacing to this GPS unit.
    - The veroboard layout is given [here](Images/miniboardvero.jpg), with copper tracks assumed underneath.
    - Connected to the main circuit board via soldered wires from J6, J7 and J8 as described above.

## SOFTWARE

Software for the MEGA 2560 microprocessor is compiled using the Arduino IDE and loaded using the USB-A to USB-B cable from the Windows 10 PC.
The main file is [MyST7920.ino](Code/MyST7920/MyST7920.ino). There are also three custom files in the same directory:

A minimal bit-bashed SPI driver for ST7920 based LCDs in [ST7920.h](Code/MyST7920/ST7920.h) and [ST7920.cpp](Code/MyST7920/ST7920.cpp). Bitmaps for fonts and a buffer used by the LCD in [bitmaps.h](Code/MyST7920/bitmaps.h). These are structured just like a library except that they reside in the same directory as the *.ino file.

The setup code:

1. Sets up interrupt servicing routines for the two interface buttons.
2. Sets up serial connections for debugging in the IDE at 56600 bps and connecting to the GPS module at 9600 bps.
3. Displays a cool startup screen for 10 seconds.
4. Setups up and displays screen 1.

The software then runs in a loop (like all Arduino code) and:

1. Responds to button presses which are originally picked up via interrupts. This is quite involved since button behaviour depends on which screen you are displaying and/or where you are in the process of editing the UTC offset.
1. Reads a block of GPS data (possibly over several passes through the loop). This consists of a series of $GP* text messages.
1. Once all the GPS data has been collected (above) into a buffer it is parsed in preperation for processing and display.
1. TIME data is error checked adjusted for offset to UTC, formatted and displayed in a way appropriate to the screen we are in.
1. DATE data is error checked adjusted for offset to UTC, formatted and displayed in a way appropriate to the screen we are in.
1. LATITUDE data is error checked formatted and displayed in a way appropriate to the screen we are in. Due to the slow LCD redraw speed this is performed only every 5th pass through the main loop.
1. LONGTITUDE data is error checked formatted and displayed in a way appropriate to the screen we are in. Due to the slow LCD redraw speed this is performed only every 5th pass through the main loop (but not the same one as LATITUDE !).
1. SPEED data is error checked formatted and displayed in a way appropriate to the screen we are in.
1. ALTITUDE data is error checked formatted and displayed in a way appropriate to the screen we are in. Due to the slow LCD redraw speed this is performed only every 5th pass through the main loop (but not the same one as LATITUDE or LONGTITUDE !).

## LIST OF PARTS

### Core Components

| Qty | Product | Description | AUD Cost | Comment | Designator |
| --- | --- | --- | --- | --- | --- |
|1| [XC3712](https://jaycar.com.au/p/XC3712) | GPS Receiver Module| $49.95 | A GPS receiver using the NEO6MV2 module, outputting NMEA data at 9600 baud. | connected to J2-1 (5V), J2-2 (GND) and TX to D19 (RX1) on MEGA |
|1| [XC4420](https://jaycar.com.au/p/XC4420) | Duinotech MEGA 2560 r3 Board | $49.95 | Arduino compatible Microcontoller brains of this project | |
|1| [XC4617](https://jaycar.com.au/p/XC4617) | 128x64 Dot Matrix LCD Display Module | $29.95 | Arduino compatible backlit graphics display based on ST920 chipset | |
|1| [ZC4821](https://jaycar.com.au/p/ZC4821) | 74HC14 Hex Schmitt trigger Inverter CMOS IC | $1.45 | Used in the hardware keypress debouncing circuit | U4 |
|1| [PI6501](https://jaycar.com.au/p/PI6501) | 14 Pin Production (Low Cost) IC Socket| $0.40 | part of hardware debounce circuit | under U4 |
|2| [RR0596](https://jaycar.com.au/p/RR0596) | 10k Ohm 0.5 Watt Metal Film Resistors - Pack of 8 ( only need 2 )| $0.85 | part of hardware debounce circuit | R2, R4 |
|2| [RR0548](https://jaycar.com.au/p/RR0548) | 100 Ohm 0.5 Watt Metal Film Resistors - Pack of 8 ( only need 2 ) | $0.85 | used in hardware debounce circuit | R1, R3 |
|2| [MCCB](https://au.element14.com/multicomp/mccb1v104m2acb/cap-0-1-f-35v-20/dp/9708480) | Tantalum Capacitor, 0.1 µF, 35 V, MCCB Series, ± 20%, Radial Leaded, 5.08 mm | $0.96 | used in hardware debounce circuit | C1, C2 |
|1| [PS0916](https://jaycar.com.au/p/PS0916) | USB TYPE A PC MOUNT Socket | $2.95 | 5.1V power for circuit | U3 |
|2| [SP0721](https://jaycar.com.au/p/SP0721) | Black Snap Action Keyboard Switch - PCB Mount | $2.90 | interface control | SW1, SW2 |
### Other Components

| Qty | Product | Description | AUD Cost | Comment | Designator |
| --- | --- | --- | --- | --- | --- |
|1| [HB6011](https://jaycar.com.au/p/HB6011) | Jiffy Box - Black - 158 x 95 x 53mm | $5.25 | enclosure of this GPS unit | |
|1| [HP9544](https://jaycar.com.au/p/HP9544) | PC Boards Vero Type Strip - 95mm x 305mm| $12.95 | cut up as required | |
|1| [WW4030](https://jaycar.com.au/p/WW4030) | Tinned Copper Wire - 100 gram Roll | $19.95 | for wiring up above Vero board | |
|1| [HM3412](https://jaycar.com.au/p/HM3412) | 2 Pin 0.1 Straight Locking Header - 2.54 pitch - Single | $0.30 | On LCD Vero board for main board connection | J3 |
|1| [HM3413](https://jaycar.com.au/p/HM3413) | 3 Pin 0.1 Straight Locking Header - 2.54 pitch - Single | $0.40 | On LCD Vero board for Arduino connection | J5 |
|1| [HM3422](https://jaycar.com.au/p/HM3422) | 2 Pin 0.1 90-degree Locking Header - 2.54 pitch - Single | $0.30 | On main board for Arduino connection | J4 |
|1 | [HM3250](https://jaycar.com.au/p/HM3250) | 40 Pin Dual In-Line (DIL) / Jumpers (only 3x2 pins) | $1.50 | for power from main board | J1, J2, J3 |
|1| [HP0924](https://jaycar.com.au/p/HP0924) | M3 x 12mm Tapped Nylon Spacers - Pk.25 (need 18)| $9.95 | For mounting Vero boards, Arduino Mega, LCD and GPS module, shorten as required| |
|1 | [HP0440](https://jaycar.com.au/p/HP0440) | M3 x 5mm Black Equipment Screws - Pk.25 (need 18)| $2.30 | as above |
|1 | [WC6026](https://jaycar.com.au/p/WC6026) | 150mm Socket to Socket Jumper Leads - 40 Piece (only need 4) | $5.95 | for connecting Vero board power to LCD and GPS modules |
|1 | [WC6028](https://jaycar.com.au/p/WC6028) | 150mm Plug to Socket Jumper Leads - 40 Piece (need 8) | $5.95 | for various plug connections to Arduino MEGA |
|1 | [WH3001](https://jaycar.com.au/p/WH3001) | Black Light Duty Hook-up Wire - 25m (only 10cm needed ) | $5.95 | for connecting mini to main Vero boards | J6, J5-1 |
|1 | [WH3004](https://jaycar.com.au/p/WH3004) | Yellow Light Duty Hook-up Wire - 25m (only 10cm needed )| $5.95 | for connecting mini to main Vero boards | J7, J5-2 |
|1 | [WH3007](https://jaycar.com.au/p/WH3007) | White Light Duty Hook-up Wire - 25m (only 10cm needed ) | $5.95 | for connecting mini to main Vero boards | J8, J5-3 |
|1 | [WC7707](https://jaycar.com.au/p/WC7707) | 0.5m USB 2.0 A Male to A Male Cable | $5.95 | for connecting external 5.1V to GPS unit via main board | U3 |
|1 | [MP3455](https://jaycar.com.au/p/MP3455) | USB Power Adaptor - 1A | $14.95 | for powering GPS unit via above cable | |

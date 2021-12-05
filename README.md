# GPS
A GPS unit with custom clock and speed screens

Based on A GPS receiver board with included antenna using the NEO6MV2 module, outputting NMEA data at 9600 baud each second. All is coordinated by a MEGA 2560 compatible microcontroller board. Information is displayed on a 128x64 LCD screen. Control is effected by just two push buttons interfaced to the microcontroller with a hardware debouncing circuit utilizing a 74HC14 Schmitt trigger IC.

There are three screens available:

The main one displays the current time, date, lattitude, longtitude, speed (SOG) and height (ASL). A calculated day of the week is shown as (m, t, w, T, f, s, S) and displayed before the date. There is a facility to edit an offset to the UTC for displaying local time. This is retained when power is off. This screen is intended for outdoor use when navigating in the great outdoors.

The second screen just displays time (UTC+adjustment) and date, with a calculated day of week shown as (Mon, Tue, Wed, Thu, Fri, Sat, Sun). Large fonts are used. This screen is intended for use as a household clock.

The third screen just displays speed over ground (SOG) in km/h. It uses a larger font and display only to the nearest km/h, ie. no decimals. This screen is intended as a vehicle speedometer.

## List of parts

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

# GPS
A GPS unit with custom clock and speed screens

Based on A GPS receiver board with included antenna using the NEO6MV2 module, outputting NMEA data at 9600 baud each second. All is coordinated by 


## List of parts
  
### Components

| Qty | Product | Description | AUD Cost | Comment | Designator____ |
| --- | --- | --- | --- | --- | --- |
|1 | [XC3712](https://jaycar.com.au/p/XC3712) | GPS Receiver Module| $49.95 | A GPS receiver using the NEO6MV2 module, outputting NMEA data at 9600 baud. | connected to J3, J5, J6 and J9 |
|1 | [XC4420](https://jaycar.com.au/p/XC4420) | Duinotech MEGA 2560 r3 Board | $49.95 | Arduino compatible Microcontoller brains of this project | U1 |
|1 | [XC4617](https://jaycar.com.au/p/XC4617) | 128x64 Dot Matrix LCD Display Module | $29.95 | Arduino compatible backlit graphics display based on ST920 chipset | B1 |
|1 | [ADA4226](https://core-electronics.com.au/adafruit-ina260-high-or-low-side-voltage-current-power-sensor.html) | Adafruit INA260 High or Low Side Voltage, Current, Power Sensor | $22.51 | measures power and voltage used by total circuit. Attach to Vero board using supplied 8 Pin Header Terminal Strip | B2 |
|1 | [ADA2652](https://core-electronics.com.au/adafruit-bme280-i2c-or-spi-temperature-humidity-pressure-sensor.html) | Adafruit BME280 I2C or SPI Temperature Humidity Pressure Sensor | $45.02 | off board sensor used in shielded I2C mode | connected to J7 |
|1 | [SEN-15901](https://core-electronics.com.au/weather-meter-kit.html) | Weather Meter Kit | $144.80 | passive reed switch based wind speed, direction and rain sensors | connected to J10 and J11 |
|1 | [BOB-14021](https://core-electronics.com.au/sparkfun-rj11-breakout.html) | SparkFun RJ11 Breakout ( currently only need 1 ) | $2.95 | contains below RJ11 Connector. Attach to Vero board using a 6 Pin Header Terminal Strip | J10 |
|2 | [PRT-00132](https://core-electronics.com.au/rj11-6-pin-connector.html) | RJ11 6-Pin Connector ( currently only need 1 )| $3.78 | attaches to above breakout board | |
|1 | [SY4058](https://jaycar.com.au/p/SY4058) | 6V DIL SPDT Mini Relay | $7.95 | used to control charging of Wifi Modem | U2 |
|1 | [YM2758](https://jaycar.com.au/p/YM2758) | Arduino Compatible 9G Micro Servo Motor | $11.95 | used to "manually" reset Wifi Modem, connect with three 2M lengths of hookup wire | connected to J4 |
|1 | [CE04421](https://core-electronics.com.au/raspberry-pi-camera-board-v2-8-megapixels-38552.html) | Raspberry Pi Camera Board v2 - 8 Megapixels | $38.95 | just for interest | connected to cable below |
|1 | [ADA1731](https://core-electronics.com.au/flex-cable-for-raspberry-pi-camera-610mm-24.html) | Flex Cable for Raspberry Pi Camera - 610mm / 24" | $6.66 | longer than standard cable for flexibility | connected to Raspberry Pi and camera
|1 | [SN74HC00N](https://au.element14.com/texas-instruments/sn74hc00n/quad-2-input-nand-74hc00-dip14/dp/3120419?st=74hc00) | Logic IC, NAND Gate, Quad, 2 Inputs, 14 Pins, DIP, 74HC00 | $0.61 | part of hardware debounce circuit for wind speed and rain sensors | connected to U5 |
|1 | [SN74HC14N](https://au.element14.com/texas-instruments/sn74hc14n/ic-hex-inverter-schmitt-74hc14/dp/9591125?st=74hc14) | Logic IC, Inverter, Hex, 1 Inputs, 14 Pins, DIP, 74HC14 | $1.03 | part of hardware debounce circuit for wind speed and rain sensors | connected to U4 |
|2 | [PI6501](https://jaycar.com.au/p/PI6501) | 14 Pin Production (Low Cost) IC Socket| $0.80 | part of hardware debounce circuit for wind speed and rain sensors | U4, U5 |
|1 | [RR0596](https://jaycar.com.au/p/RR0596) | 10k Ohm 0.5 Watt Metal Film Resistors - Pack of 8 ( only need 3 )| $0.85 | part of hardware debounce and input circuit for Weather Meter Kit sensors | R3, R5, R6 |
|1 | [RR0572](https://jaycar.com.au/p/RR0572) | 1k Ohm 0.5 Watt Metal Film Resistors - Pack of 8 ( only need 3 ) | $0.85 | used in hardware debounce and relay circuits | R1, R2, R4 |
|1 | [ZT2152](https://jaycar.com.au/p/ZT2152) | BC547 NPN Transistor | $0.30 | used in relay circuit | Q1 |
|1 | [ZR1004](https://jaycar.com.au/p/ZR1004) | 1N4004 1A 400V Diode - Pack of 4 ( only need 1 )| $0.98 | used in relay circuit | D1 |
|2 | [MCCB](https://au.element14.com/multicomp/mccb1v104m2acb/cap-0-1-f-35v-20/dp/9708480) | Tantalum Capacitor, 0.1 µF, 35 V, MCCB Series, ± 20%, Radial Leaded, 5.08 mm | $4.34 | used in hardware debounce circuit for wind speed and rain sensors | C1, C2 |

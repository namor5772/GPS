// IN GITHUB
// C:\Users\roman\Documents\GitHub\GPS\Code\MyST7920
//

#include "ST7920.h" // minimal bit-bashed SPI drivers for ST7920 based LCD
#include "bitmaps.h" // font and buffer bitmaps used by LCD
#include <EEPROM.h> // saving data between reboots

#define BMAX 1024 // max size for buffer
#define BSML 16 // small buffer size
#define GPSM 85 // millisecond gap for saving gps message, 80 is the minimum stable level.
#define SECA 2 // reduced decimal places for seconds of position
#define SCHR 32 // char to replace blank in NA message (95 = underline)
#define SCREEN_NUM 3 // number of user screens

// create LCD screen object
ST7920 LCD = ST7920(60);

bool gpsC; // gps data collection flag
volatile unsigned long tn; // time now
char gps[BMAX]; // buffer for gps data
int i, j, k, m, n;
char b, c, d;
bool GOODdata;
uint8_t LoopNum = 0;

// variables for token parsing
int NEMAtokenNum;
bool NEMAparse;
char NEMAtoken[BSML];

// variables for GPRMC message
bool GPRMCmsg;
char GPRMC[BSML] = "GPRMC";

// variables for GPGGA message
bool GPGGAmsg;
char GPGGA[BSML] = "GPGGA";

// SOME GLOBAL VARIABLES START - CAN BE MOVED TO LOCAL!
    char TimeFix[BSML]; // token 1
    uint8_t TimeFix1[BSML]; // to store final string for printing
    uint8_t TimeFix1OLD[BSML]; // to store previous copy of final string
    int tf, tf1, hr1, hr0, min1, min0, sec1, sec0; 
    
    char Latitude1[BSML], Latitude2[BSML]; // tokens 3 & 4
    uint8_t Latitude[BSML]; // to store final string for printing
    uint8_t LatitudeOLD[BSML]; // to store previous copy of final string
    
    char Longtitude1[BSML], Longtitude2[BSML]; // tokens 5 & 6
    uint8_t Longtitude[BSML]; // to store final string for printing
    uint8_t LongtitudeOLD[BSML]; // to store previous copy of final string
    
    char SpeedN[BSML]; // token 7
    char SpeedK[BSML]; // stores SOG in km/h in null-terminated char array
    uint8_t SpeedK2[BSML]; // to store final string for printing
    uint8_t SpeedK2OLD[BSML]; // to store previous copy of final string
    
    char DateFix[BSML]; // token 9
    uint8_t DateFix1[BSML]; // to store final string for printing
    uint8_t DateFix1OLD[BSML]; // to store previous copy of final string
    int DayOfs; // -1=previous, 0=current, +1=next day/ 
    int mth1, mth0, day1, day0, m2, m1, m0, yr1, yr0, d1, d2, d0, dw;
    
    char AltitudeM[BSML]; // token 9
    char AltitudeM2[BSML]; // (re)stores ASL in metres in formatted char array
    uint8_t AltitudeM3[BSML]; // to store final string for printing
    uint8_t AltitudeM3OLD[BSML]; // to store previous copy of final string
// GLOBAL VARIABLES END

// button interrupt related variables
const byte interruptPinA = 2;
const byte interruptPinB = 3;
volatile byte stateA = LOW;
volatile uint8_t NumA = 0; 
volatile byte stateB = LOW;
volatile uint8_t NumB = 0; 

// button related variables
uint8_t Bpos = 0;
uint8_t Bchar = 32;
uint8_t TimeAdj[4]; // stores hour offset to UTC in characters
uint8_t TimeAdjS[4]; // stores hour offset to UTC in "scroll-mapping" format
int TimeAdjN; // stores hour offset to UTC in 2*value format (since never finer that 1/2's)
int EEPROM_TimeAdjN = 0; // EEPROM address for TimeAdjN (uses 2 bytes)
uint8_t ColOfs; // column currently in focus
uint8_t ColCursor[4]; // stores cursor graphics under edit field
uint8_t ScreenID; // current screen chosen
bool Redraw[7]; // array of various redraw flags
int EditFields[] = {5,1,1}; // number of edit fields in each screen 0, 1 and 2


// button A pressed interrupt
void isrA() {
    Serial.print(" ***** BUTTON A PRESSED *****\n");
    stateA = HIGH;
    NumA++;
}

// button B pressed interrupt
void isrB() {
    Serial.print(" ***** BUTTON B PRESSED *****\n");
    stateB = HIGH;
    NumB++;
}

// In Screen0 used to map scroll position to character
char Screen0_Map_S2C(uint8_t Bpos) {
    char c;
    switch (Bpos) {
        case 0:  c=32; break; // ' '
        case 1:  c=48; break; // '0'
        case 2:  c=49; break; // '1'
        case 3:  c=50; break; // '2'
        case 4:  c=51; break; // '3' 
        case 5:  c=52; break; // '4' 
        case 6:  c=53; break; // '5' 
        case 7:  c=54; break; // '6' 
        case 8:  c=55; break; // '7' 
        case 9:  c=56; break; // '8' 
        case 10: c=57; break; // '9' 
        case 11: c=43; break; // '+' 
        case 12: c=45; break; // '-'
        case 13: c=157; break; // '1/2'
        default: c=63; break; // '?' - do nothing                                 
    }
    return c;
}

// In Screen0 used to map scroll position to charcter
uint8_t Screen0_Map_C2S(uint8_t Bchar) {
    uint8_t s;
    switch (Bchar) {
        case 32:  s=0; break; // ' '
        case 43: s=11; break; // '+' 
        case 45: s=12; break; // '-'
        case 48:  s=1; break; // '0'
        case 49:  s=2; break; // '1'
        case 50:  s=3; break; // '2'
        case 51:  s=4; break; // '3' 
        case 52:  s=5; break; // '4' 
        case 53:  s=6; break; // '5' 
        case 54:  s=7; break; // '6' 
        case 55:  s=8; break; // '7' 
        case 56:  s=9; break; // '8' 
        case 57: s=10; break; // '9' 
        case 157: s=13; break; // '1/2'
        default: s=0; break; // do nothing                                 
    }
    return s;
}

// uses bytes at [address, address+1] to store int whch is assumed to use 2 bytes
void writeIntIntoEEPROM(int address, int number) { 
  byte byte1 = number >> 8;
  byte byte2 = number & 0xFF;
  EEPROM.write(address, byte1);
  EEPROM.write(address + 1, byte2);
}

// uses bytes at [address, address+1] to extract 2 byte (assumed) int
int readIntFromEEPROM(int address) {
  byte byte1 = EEPROM.read(address);
  byte byte2 = EEPROM.read(address + 1);
  return (byte1 << 8) + byte2;
}

// takes time adjustment value TimeAdjN and harmoinizes with the assumed size 4 arrays TimeAdj & TimeAdj
void HarmioniseTimeVars(int xTimeAdjN, uint8_t * xTimeAdj, uint8_t * xTimeAdjS) {
    int n;
    char str[BSML]; // temp char string used for internal calculations

    // create time adjustment offset in hours as 'x'
    float x=((float)xTimeAdjN)/2.0;

    // using 'x' recalculate all offset variables
    dtostrf(x,4,1,str);
    for (n=0; n<4; n++) xTimeAdj[n]=32; // clear xTimeAdj in prep for rewrite
    int m=0; do {m++;} while (str[m]!=0); m--; // get length of str (converted version of x)

    // re calculating xTimeAdj[], If necessary replace '.5' with '1/2' and
    // ignoring '.0', also ignoring leading '0' and prepending '+' to positive numbers.
    if (str[m]=='5') {
        // dealing with a half
        xTimeAdj[3] = 157;
        xTimeAdj[2] = str[m-2];                                                    
        xTimeAdj[1] = str[m-3];
        if (m>3) xTimeAdj[0] = str[m-4]; }
    else if (str[m]=='0') {
        xTimeAdj[3] = str[m-2];
        xTimeAdj[2] = str[m-3];  
        if (m>3) xTimeAdj[1] = str[m-4];
        if (m>4) xTimeAdj[0] = str[m-5]; }
    
    // find first '0' in xTimeAdj (if any), and replace it with a blank.
    n=0; while (xTimeAdj[n]==32) n++; if ((xTimeAdj[n]==48)&&(n<3)) xTimeAdj[n]=32;

    // find first non blank character in xTimeAdj. If it is not '-' prepend it with '+'.
    n=0; while (xTimeAdj[n]==32) n++; if (xTimeAdj[n]!=45) xTimeAdj[n-1]=43;

    // 'Synchronize' xTimeAdjS and TimeAdjN with xTimeAdj
    for (n=0; n<4; n++) xTimeAdjS[n]=Screen0_Map_C2S(xTimeAdj[n]); 
}

// Takes the 32x32 char numbered CharNum (0,..,10) from the big font char array f and copies it to position
// pos (0,1,2) in the p array (32x96 bits = 3 32x32 blocks)
void PrintBigChar(int CharNum, int pos, unsigned char *p, unsigned char *f) {
    int ix, iy;
    for (iy=0; iy<32; iy++) {
        for (ix=0; ix<4; ix++) {
            p[iy*12+((4*pos)+ix)] = f[iy*44+(4*CharNum)+ix];
        }
    }
}

// day of week (0,..,6) with 0=Mon. Works till Sunday 28-Feb-2100 inclusive.
// Source: Calendrical Calculations, The Ultimate Edition.
int day_of_week(int xyear, int xmth, int xday) {
    long x;
    if (xmth<=2) {x=0;} else if (xyear%4==0){x=-1;} else {x=-2;}
    return (int)(((365*((long)xyear+1999))+(((long)xyear+1999)/4)+((367*(long)xmth-362)/12)+x+xday-738124)%7);
}

void setup() {
    // setup button press interrupts
    pinMode(interruptPinA, INPUT);
    pinMode(interruptPinB, INPUT);
    attachInterrupt(digitalPinToInterrupt(interruptPinA), isrA, RISING);
    attachInterrupt(digitalPinToInterrupt(interruptPinB), isrB, RISING);

    // setup serial connections
    Serial.begin(57600);
    Serial1.begin(9600);

    // display startup screen for 10 seconds (We Are Legion!)
    LCD.InitDriverPins(52,53,51); // D52=E=SCK, D53=RS=SS, D51=R/W=MOSI (here 9,7,8 bit-bashed) 
    LCD.Init();  
    LCD.DrawFullScreen(StartupScreen);
    delay(10000);
    LCD.Clear(); 
    
    ScreenID = 0;
    switch (ScreenID) {
        case 0:
            // load time adjustment variable TimeAdjN from EEPROM and 
            // harmonize it with related variables then display it and cursor line 
            ColOfs = 0;
            TimeAdjN = readIntFromEEPROM(EEPROM_TimeAdjN);
            if (abs(TimeAdjN)>47) TimeAdjN=0; // in case there are problems reading EEPROM
            HarmioniseTimeVars(TimeAdjN, TimeAdj, TimeAdjS);
            for (n=0; n<3; n++) ColCursor[n]=32; ColCursor[3]=7;
            for (m=0; m<2; m++) LCD.Draw8x16(m+6,0*8,TimeAdj[2*m],TimeAdj[2*m+1],f);
            for (m=0; m<2; m++) LCD.Draw8x16(m+6,1*8,ColCursor[2*m],ColCursor[2*m+1],f);
            
            // draw SOG line fixed text
            LCD.Draw8x16(0,6*8,83,79,f);
            LCD.Draw8x16(1,6*8,71,32,f);
            LCD.Draw8x16(5,6*8,107,109,f);
            LCD.Draw8x16(6,6*8,47,104,f);
            
            // draw ASL line fixed text
            LCD.Draw8x16(0,7*8,65,83,f);
            LCD.Draw8x16(1,7*8,76,32,f);
            LCD.Draw8x16(6,7*8,109,32,f);

            // initialise redraw flags to false
            for (n=0; n<7; n++) Redraw[n]=false;
            break;
        case 1:
            // draw time line fixed text
            LCD.Draw24x16(2,24,10,SmallerFont);
            LCD.Draw24x16(5,24,10,SmallerFont);
        
            break;
        case 2:
            break;
    }        
}

void loop() {    
    if (stateA==HIGH) {
        Serial.print(" *** ACT ON BUTTON A PRESS "); Serial.print(NumA); Serial.print(" TIMES ***\n");
        ColOfs = (NumA+ColOfs)%EditFields[ScreenID];
        
        // reached "off-edit area" state, so process screen edits depending Screen
        switch (ScreenID) {
            case 0:
                if (ColOfs==0) {
                    Serial.print(" PROCESS SCREEN EDITS\n");
                    // process edits on Screen 0
                    
                    // this means updating TimeAdjN (the time adjustment value) and the related
                    // TimeAdj & TimeAdjS uint8_t arrays (used for displaying/editing TimeAdjN)
                    // which need to be synchronized as a 'package'.
    
                    // convert time offset string TimeAdj into temporary str (usable by the strtod) 
                    char str[BSML];
                    char *ptr;
                    float ret, x;
                    int ofs;
                    ofs = 0;
                    for (m=0; m<4; m++) {
                        if (TimeAdj[m]!=157) {str[m+ofs] = TimeAdj[m]; }
                        else {str[m+ofs] = '.'; str[m+ofs+1] = '5'; ofs++; } }
                    str[4+ofs] = 0;
                    
                    // apply strtod to str (converted from TimeAdj)
                    ret = strtod(str, &ptr);
    
                    // time offset should be an integer if multiplied by 2 and its absolute value
                    // not greater than 23.5. If not then set 'ret' to 0.
                    x = ret*2.0;
                    if ((fabsf(roundf(x)-x) > 0.0001f)||(fabsf(x)>47.0)) ret=0.0;
                    TimeAdjN = (int)(ret*2);
    
                    // harmonize the variables TimeAdj and TimeAdjS related to TimeAdjN
                    HarmioniseTimeVars(TimeAdjN, TimeAdj, TimeAdjS);
                    
                    // Save TimeAdjN to EEPROM. This will be used on startup to reconstruct
                    // TimeAdj and TimeAdjS, and display value on Screen0
                    writeIntIntoEEPROM(EEPROM_TimeAdjN, TimeAdjN);
    
                    // display TimeAdj
                    for (m=0; m<2; m++) LCD.Draw8x16(m+6,0*8,TimeAdj[2*m],TimeAdj[2*m+1],f);
    
                    // setup to draw 'off-edit-field' indicator under edit field
                    for (n=0; n<3; n++) ColCursor[n]=32; ColCursor[3]=7; 
                    
                }
                else { // if ColOfs>0
                    // setup to draw cursor under edited char
                    for (n=0; n<4; n++) {ColOfs==n+1 ? ColCursor[n]=143 : ColCursor[n]=32;}
                }
                // do actual drawing of characters under edit field
                for (m=0; m<2; m++) LCD.Draw8x16(m+6,1*8,ColCursor[2*m],ColCursor[2*m+1],f);
                break;
                
            default: // do nothing
                break;                    
        }
        NumA=0;
        stateA=LOW;
    }

    if (stateB==HIGH) {
        Serial.print(" *** ACT ON BUTTON B PRESS "); Serial.print(NumB); Serial.print(" TIMES ***\n");
        if (ColOfs==0) {
            // increment screen being viewed, this involves lots of redrawing etc.
            ScreenID = (ScreenID+NumB)%SCREEN_NUM;
            switch (ScreenID) {
                case 0:
                    Serial.print(" INCREMENTED to SCREEN 0\n");
                    LCD.Clear(); 
                    
                    // draw SOG line fixed text
                    LCD.Draw8x16(0,6*8,83,79,f);
                    LCD.Draw8x16(1,6*8,71,32,f);
                    LCD.Draw8x16(5,6*8,107,109,f);
                    LCD.Draw8x16(6,6*8,47,104,f);
                    
                    // draw ASL line fixed text
                    LCD.Draw8x16(0,7*8,65,83,f);
                    LCD.Draw8x16(1,7*8,76,32,f);
                    LCD.Draw8x16(6,7*8,109,32,f);

                    // display TimeAdj then display it and cursor line 
                    for (n=0; n<3; n++) ColCursor[n]=32; ColCursor[3]=7;
                    for (m=0; m<2; m++) LCD.Draw8x16(m+6,0*8,TimeAdj[2*m],TimeAdj[2*m+1],f);
                    for (m=0; m<2; m++) LCD.Draw8x16(m+6,1*8,ColCursor[2*m],ColCursor[2*m+1],f);
                    break;
                    
                case 1:
                    Serial.print(" INCREMENTED to SCREEN 1\n");
                    LCD.Clear();
                     
                    // draw time line fixed text
                    LCD.Draw24x16(2,12,10,SmallerFont);
                    LCD.Draw24x16(5,12,10,SmallerFont);
                    
                    break;
                    
                case 2:
                    Serial.print(" INCREMENTED to SCREEN 2\n");
                    LCD.Clear(); 
                    break;
                    
                default: // do nothing
                    break;                    
            }
            // Set Redraw flag
            for (n=0; n<7; n++) Redraw[n]=true;
        }
        else {
            // action depends on screen you are in
            switch (ScreenID) {
                case 0:
                    TimeAdjS[ColOfs-1] = (NumB+TimeAdjS[ColOfs-1])%14;
                    TimeAdj[ColOfs-1] = Screen0_Map_S2C(TimeAdjS[ColOfs-1]);
                    for (m=0; m<2; m++) LCD.Draw8x16(m+6,0*8,TimeAdj[2*m],TimeAdj[2*m+1],f);
                    break;
                default: // do nothing
                    break;                    
            }
        }        
        NumB=0;
        stateB=LOW;
        Serial.print(" ColOfs = "); Serial.print(ColOfs); Serial.print("\n");
    }
    
    // Read a block of GPS data, consisting of a series of $GP* text messages 
    i=0;
    gpsC= false;
    tn = millis();
    while (millis() < tn+GPSM) { 
        while(Serial1.available()) {
            c = Serial1.read();
            if ((c=='$')&&(!gpsC)) {
                // start saving data in buffer, before parsing
                gpsC = true;
                tn = millis();
            }
            else if (c=='$') {
                // continue saving data in buffer for another GPSM milliseconds
                tn = millis();
            }
            
            // actually collect these bytes
            if (gpsC) {
                // PRINT OUT BLOCK OF GPS DATA                                     
                Serial.write(c);
                gps[i++] = c;
            }
        }                
    }
    Serial.print(" ----- "); Serial.print(i); Serial.print(" elements in buffer\n");
    
    // A block of GPS data has been collected into the gps buffer.
    // It can now be processed and acted upon as appropriate
    if (gpsC) {
        // increment the global loop variable (used to reduce display load)
        // this is done ONLY for iterations which get valid gps data! 
        LoopNum++; 
        Serial.print(LoopNum); Serial.print(" Valid gps data loop iteration\n");
        
        // resetflags after gps data has been collected
        NEMAparse = false;
        NEMAtokenNum = 0;
        GPRMCmsg = false;
        GPGGAmsg = false;
        j=0; k=0;

        // process gps data block by seqentially working your way through its characters,
        // seems simple but is actually quite messy
        do {
            b = gps[j++]; // get next character
            if (b=='$') {
                // start of NEMA message
                NEMAparse = true;
                NEMAtokenNum = 0;
                k = 0;
            }
            else if (NEMAparse) {
                if (b==',') {
                    // reached end of comma delimited token
                    if (NEMAtokenNum==0) {
                        // end of NEMA message type
                        NEMAtoken[k]=0; // delimit it 

                        // do stuff with token
                        if (strcmp(NEMAtoken,GPRMC)==0) { // dealing with GPRMC NEMA message
                            GPRMCmsg = true;
                        } else if (strcmp(NEMAtoken,GPGGA)==0) { // dealing with GPGGA NEMA message
                            GPGGAmsg = true;
                        }

                        NEMAtokenNum++; // parsing next token
                        k = 0;
                    }
                    else if (NEMAtokenNum>0) {
                        NEMAtoken[k]=0; // delimit it 

                        if (GPRMCmsg) {
                            // do stuff specific to GPRMC message
                            switch (NEMAtokenNum) {
                                case 1: strcpy(TimeFix,NEMAtoken); break;
                                case 3: strcpy(Latitude1,NEMAtoken); break;
                                case 4: strcpy(Latitude2,NEMAtoken); break;
                                case 5: strcpy(Longtitude1,NEMAtoken); break;
                                case 6: strcpy(Longtitude2,NEMAtoken); break;
                                case 7: strcpy(SpeedN,NEMAtoken); break;
                                case 9: strcpy(DateFix,NEMAtoken); break;
                                default: break; // do nothing                                 
                            }
                        } else if (GPGGAmsg) {
                            // do stuff specific to GPGGA message
                            switch (NEMAtokenNum) {
                                case 9: strcpy(AltitudeM,NEMAtoken); break;
                                default: break; // do nothing                                                              
                            }
                        }
                        NEMAtokenNum++; // parsing next token
                        k = 0;
                    }
                } 
                else if (b==13) {
                    // reached last token of currently parsed NEMA message
                    // can assume NEMAtoken>0
                    NEMAparse = false;
                    NEMAtoken[k]=0; // delimit it 
                    if (GPRMCmsg) { // do stuff specific to GPRMC message
                        GPRMCmsg = false;
                    }
                    else if (GPGGAmsg) { // do stuff specific to GPGGA message
                        GPGGAmsg = false;
                    }
                    NEMAtokenNum++; // parsing next token
                    k = 0;
                } 
                else {
                    // this prevents buffer overflow if invalid data is being parsed
                    if (k<BSML) NEMAtoken[k++] = b;
                }
            }
        } while (b!=0);
        for (k=0; k<BMAX; k++) gps[k]=0; // clear gps data buffer       


        // ***** TIME *****
        // In ROW1 display time in hh:mm:ss format
        // check data for validity ie. 0,...,9
        GOODdata = true;
        for (k=0; k<6; k++) {tf=(int)TimeFix[k]; if ((tf>57)||(tf<48)) GOODdata = false;}
                    
        // display time if data valid else NA message
        if (GOODdata) {
            // adjust TimeFix by TimeAdj (so we can display local time)
            // create adjusted time of day for current UTC (in TimeFix char array)

            // extract hr & min integers from the TimeFix char array
            int xhr, xmin;
            char tmp[BSML];
            tmp[0]=TimeFix[0]; tmp[1]=TimeFix[1]; tmp[2]=0; xhr=atoi(tmp);
            tmp[0]=TimeFix[2]; tmp[1]=TimeFix[3]; tmp[2]=0; xmin=atoi(tmp);

            // modify hr & min by TimeAdj, also DayOfs.
            DayOfs=0; // default
            int dmin = xhr*60+xmin; // minute of UTC day
            int amin = (dmin+TimeAdjN*30); // adjusted minute of day
            if ((TimeAdjN>0)&&(amin>=1440)) {
                amin -= 1440; DayOfs = 1; }
            else if ((TimeAdjN<0)&&(amin<0)) {
                amin += 1440; DayOfs = -1; }
            xhr = amin/60;
            xmin = amin%60;

            // convert new hr & min to updated TimeFix char array
            itoa(xhr,tmp,10);
            if (tmp[1]==0) {TimeFix[0]=48; TimeFix[1]=tmp[0];} else {TimeFix[0]=tmp[0]; TimeFix[1]=tmp[1];}
            itoa(xmin,tmp,10);
            if (tmp[1]==0) {TimeFix[2]=48; TimeFix[3]=tmp[0];} else {TimeFix[2]=tmp[0]; TimeFix[3]=tmp[1];}

            // extract characters from TimeFix and create formatted version TimeFix1
            hr1 = (int)TimeFix[0];
            hr0 = (int)TimeFix[1];
            min1 = (int)TimeFix[2];
            min0 = (int)TimeFix[3];
            sec1 = (int)TimeFix[4];
            sec0 = (int)TimeFix[5];
            TimeFix1[0] = hr1; TimeFix1[1] = hr0; 
            TimeFix1[2] = 58;
            TimeFix1[3] = min1; TimeFix1[4] = min0; 
            TimeFix1[5] = 58;
            TimeFix1[6] = sec1; TimeFix1[7] = sec0; 

            if (ScreenID==0) {
                // display time in 8 char row starting at column 1 (using TimeFix1)
                // by comparing TimeFix1 with TimeFix1OLD and only redrawing changed 2 char blocks
                if (!Redraw[0]) {
                    for (k=0; k<4; k++) {
                        if ((TimeFix1[2*k]!=TimeFix1OLD[2*k])||(TimeFix1[2*k+1]!=TimeFix1OLD[2*k+1])) {
                            LCD.Draw8x16(k,0*8,TimeFix1[2*k],TimeFix1[2*k+1],f);
                        }
                        // save TimeFix1 to TimeFix1OLD for comparison at next redraw
                        TimeFix1OLD[2*k]=TimeFix1[2*k];
                        TimeFix1OLD[2*k+1]=TimeFix1[2*k+1];
                    }                
                }
                else { // do full redraw and set redraw flag to false.
                    for (k=0; k<4; k++) {
                        LCD.Draw8x16(k,0*8,TimeFix1[2*k],TimeFix1[2*k+1],f);
                        // save TimeFix1 to TimeFix1OLD for comparison at next redraw
                        TimeFix1OLD[2*k]=TimeFix1[2*k];
                        TimeFix1OLD[2*k+1]=TimeFix1[2*k+1];
                    }   
                    Redraw[0]=false;                     
                }
            }                
            else if (ScreenID==1) {
                m=0;
                // display time in 8 char row starting at column 3 (using TimeFix1)
                // by comparing TimeFix1 with TimeFix1OLD and only redrawing changed 2 char blocks
                if (!Redraw[0]) {
                    for (k=0; k<8; k++) {
                        if (TimeFix1[k]!=TimeFix1OLD[k]) LCD.Draw24x16(k,12,TimeFix1[k]-48,SmallerFont);
                        // save TimeFix1 to TimeFix1OLD for comparison at next redraw
                        TimeFix1OLD[k]=TimeFix1[k];
                    }                
                }
                else { // do full redraw and set redraw flag to false.
                    for (k=0; k<8; k++) {
                        LCD.Draw24x16(k,12,TimeFix1[k]-48,SmallerFont);
                        // save TimeFix1 to TimeFix1OLD for comparison at next redraw
                        TimeFix1OLD[k]=TimeFix1[k];
                    }   
                    Redraw[0]=false;                     
                }
            }
            else if (ScreenID==2) {
                ;                
            }

        }
        else {
            if (ScreenID==0) {
                // NA message
                LCD.Draw8x16(0,0*8,78,65,f);
            }                
            else if (ScreenID==1) {
                ;                
            }
            else if (ScreenID==2) {
                ;                
            }
        }


        // ***** DATE *****
        // In ROW2 display date in dd-mmm-yyyy format
        // check data for validity ie. 0,...,9
        GOODdata = true;
        for (k=0; k<6; k++) {tf=(int)DateFix[k];if ((tf>57)||(tf<48)) GOODdata = false;}            

        // display date if data valid else NA message
        if (GOODdata&&((LoopNum+0)%5==0)) {
            // adjust DateFix by DayOfs (so we can display local time)
            // create adjusted date for current UTC (in DateFix char array)
            
            // extract day, month and ear integers from the DateFix char array
            int xdw, xday, xmth, xyear; // xdw = day of week (0=Mon,..,6=Sun)
            char tmp[BSML];
            tmp[0]=DateFix[0]; tmp[1]=DateFix[1]; tmp[2]=0; xday=atoi(tmp);
            tmp[0]=DateFix[2]; tmp[1]=DateFix[3]; tmp[2]=0; xmth=atoi(tmp);
            tmp[0]=DateFix[4]; tmp[1]=DateFix[5]; tmp[2]=0; xyear=atoi(tmp);

            // modify day, month & year by DayOfs.
            switch (DayOfs) {
                case -1:
                    // go one day back from first date of month:
                    if ((xday==1)&&(xmth==1)) {xday=31; xmth=12;} // Jan
                    else if ((xday==1)&&(xmth==2)) {xday=31; xmth=1;} // Feb
                    else if ((xday==1)&&(xmth==3)) {xmth=2; (xyear%4==0)?xday=29:xday=28;} //Mar
                    else if ((xday==1)&&(xmth==4)) {xday=31; xmth=3;} // Apr
                    else if ((xday==1)&&(xmth==5)) {xday=30; xmth=4;} // May
                    else if ((xday==1)&&(xmth==6)) {xday=31; xmth=5;} // Jun
                    else if ((xday==1)&&(xmth==7)) {xday=30; xmth=6;} // Jul
                    else if ((xday==1)&&(xmth==8)) {xday=31; xmth=7;} // Aug
                    else if ((xday==1)&&(xmth==9)) {xday=31; xmth=8;} // Sep
                    else if ((xday==1)&&(xmth==10)) {xday=30; xmth=9;} // Oct
                    else if ((xday==1)&&(xmth==11)) {xday=31; xmth=10;} // Nov
                    else if ((xday==1)&&(xmth==12)) {xday=30; xmth=11;} // Dec
                    // go one day backwards within month
                    else {xday--;};
                    break;
                case 1:
                    // go one day forward from last date of month:
                    if ((xday==31)&&(xmth==1)) {xday=1; xmth=2;} // Jan
                    else if ((xday==29)&&(xmth==2)) {xday=1; xmth=3;} // Feb - on definite leap year
                    else if ((xday==28)&&(xmth==2)) { // Feb
                        if (xyear%4==0) {xday=29; xmth=2;}// leap year (ignore 2100!)
                        else {xday=1; xmth=3;} }
                    else if ((xday==31)&&(xmth==3)) {xday=1; xmth=4;} // Mar
                    else if ((xday==30)&&(xmth==4)) {xday=1; xmth=5;} // Apr
                    else if ((xday==31)&&(xmth==5)) {xday=1; xmth=6;} // May
                    else if ((xday==30)&&(xmth==6)) {xday=1; xmth=7;} // Jun
                    else if ((xday==31)&&(xmth==7)) {xday=1; xmth=8;} // Jul
                    else if ((xday==31)&&(xmth==8)) {xday=1; xmth=9;} // Aug
                    else if ((xday==30)&&(xmth==9)) {xday=1; xmth=10;} // Sep
                    else if ((xday==31)&&(xmth==10)) {xday=1; xmth=11;} // Oct
                    else if ((xday==30)&&(xmth==11)) {xday=1; xmth=12;} // Nov
                    else if ((xday==31)&&(xmth==12)) {xday=1; xmth=1; xyear++;} // Dec - inc year
                    // go one day forward within month
                    else {xday++;};
                    break;
                default:
                    break; // do nothing
            }
            
            // convert new day, month & year to updated DateFix char array
            itoa(xday,tmp,10);
            if (tmp[1]==0) {DateFix[0]=48; DateFix[1]=tmp[0];} else {DateFix[0]=tmp[0]; DateFix[1]=tmp[1];}
            itoa(xmth,tmp,10);
            if (tmp[1]==0) {DateFix[2]=48; DateFix[3]=tmp[0];} else {DateFix[2]=tmp[0]; DateFix[3]=tmp[1];}
            itoa(xyear,tmp,10);
            if (tmp[1]==0) {DateFix[4]=48; DateFix[5]=tmp[0];} else {DateFix[4]=tmp[0]; DateFix[5]=tmp[1];}
            
            // extract characters from DateFix and create formatted version DateFix1
            day1 = (int)DateFix[0];
            day0 = (int)DateFix[1];
            mth1 = (int)DateFix[2];
            mth0 = (int)DateFix[3];
            yr1 = (int)DateFix[4];
            yr0 = (int)DateFix[5];
            switch ((mth1-48)*10+(mth0-48)) {
                case 1:  m2=74; m1=97;  m0=110; break; // Jan
                case 2:  m2=70; m1=101; m0=98;  break; // Feb
                case 3:  m2=77; m1=97;  m0=114; break; // Mar
                case 4:  m2=65; m1=112; m0=114; break; // Apr
                case 5:  m2=77; m1=97;  m0=121; break; // May
                case 6:  m2=74; m1=117; m0=110; break; // Jun
                case 7:  m2=74; m1=117; m0=108; break; // Jul
                case 8:  m2=65; m1=117; m0=103; break; // Aug
                case 9:  m2=83; m1=101; m0=112; break; // Sep
                case 10: m2=79; m1=99;  m0=116; break; // Oct
                case 11: m2=78; m1=111; m0=118; break; // Nov
                case 12: m2=68; m1=101; m0=99; break;  // Dec
                default: m2=63; m1=63;  m0=63; break;  // Error
            }

            // calculate day of week xdw (using xday, xmth and xyear)
            xdw = day_of_week(xyear, xmth, xday);
            
            if (ScreenID==0) {
                // determine day of week text (m,t,w,T,f,s,S)
                switch (xdw) {
                    case 0:  dw=109; break; // Mon - m
                    case 1:  dw=116; break; // Tue - t
                    case 2:  dw=119; break; // Wed - w
                    case 3:  dw=84;  break; // Thu - T
                    case 4:  dw=102; break; // Fri - f
                    case 5:  dw=115; break; // Sat - s
                    case 6:  dw=83;  break; // Sun - S
                    default: dw=69;  break; // ERR - E
                }
                
                DateFix1[0] = dw;
                DateFix1[1] = day1; DateFix1[2] = day0;
                DateFix1[3] = 45;
                DateFix1[4] = m2; DateFix1[5] = m1; DateFix1[6] = m0;
                DateFix1[7] = 45;
                DateFix1[8] = 50; DateFix1[9] = 48; DateFix1[10] = yr1; DateFix1[11] = yr0;
                
                // display time in 12 char row starting at column 1 (using DateFix1)
                // by comparing DateFix1 with DateFix1OLD and only redrawing changed 2 char blocks
                if (!Redraw[1]) {
                    for (k=0; k<6; k++) {
                        if ((DateFix1[2*k]!=DateFix1OLD[2*k])||(DateFix1[2*k+1]!=DateFix1OLD[2*k+1])) {
                            LCD.Draw8x16(k,1*8,DateFix1[2*k],DateFix1[2*k+1],f); }
                        // save DateFix1 to DateFix1OLD for comparison at next redraw
                        DateFix1OLD[2*k]=DateFix1[2*k];
                        DateFix1OLD[2*k+1]=DateFix1[2*k+1];
                    }                
                }
                else { // do full redraw and set redraw flagf to false.
                    for (k=0; k<6; k++) {
                        LCD.Draw8x16(k,1*8,DateFix1[2*k],DateFix1[2*k+1],f);
                        // save DateFix1 to DateFix1OLD for comparison at next redraw
                        DateFix1OLD[2*k]=DateFix1[2*k];
                        DateFix1OLD[2*k+1]=DateFix1[2*k+1];
                    }                
                    Redraw[1]=false;                     
                }
            }    
            else if (ScreenID==1) {
                // determine day of week text
                switch (xdw) {
                    case 0:  d2=77; d1=111; d0=110; break; // Mon
                    case 1:  d2=84; d1=117; d0=101;  break; // Tue
                    case 2:  d2=87; d1=101; d0=100; break; // Wed
                    case 3:  d2=84; d1=104; d0=117; break; // Thu
                    case 4:  d2=70; d1=114; d0=105; break; // Fri
                    case 5:  d2=83; d1=97;  d0=116; break; // Sat
                    case 6:  d2=83; d1=117; d0=110; break; // Sun
                    default: d2=69; d1=82;  d0=82; break;  // ERR
                }
                
                DateFix1[0] = d2;
                DateFix1[1] = d1;
                DateFix1[2] = d0;
                DateFix1[3] = 32;
                DateFix1[4] = day1; DateFix1[5] = day0;
                DateFix1[6] = 45;
                DateFix1[7] = m2; DateFix1[8] = m1; DateFix1[9] = m0;
                DateFix1[10] = 45;
                DateFix1[11] = 50; DateFix1[12] = 48; DateFix1[13] = yr1; DateFix1[14] = yr0;
                DateFix1[15] = 32;
                
                // display time in 12 char row starting at column 1 (using DateFix1)
                // by comparing DateFix1 with DateFix1OLD and only redrawing changed 2 char blocks
                if (!Redraw[1]) {
                    for (k=0; k<8; k++) {
                        if ((DateFix1[2*k]!=DateFix1OLD[2*k])||(DateFix1[2*k+1]!=DateFix1OLD[2*k+1])) {
                            LCD.Draw8x16(k,6*8,DateFix1[2*k],DateFix1[2*k+1],f);
                        }
                        // save DateFix1 to DateFix1OLD for comparison at next redraw
                        DateFix1OLD[2*k]=DateFix1[2*k];
                        DateFix1OLD[2*k+1]=DateFix1[2*k+1];
                    }                
                }
                else { // do full redraw and set redraw flaf to false.
                    for (k=0; k<8; k++) {
                        LCD.Draw8x16(k,6*8,DateFix1[2*k],DateFix1[2*k+1],f);
                        // save DateFix1 to DateFix1OLD for comparison at next redraw
                        DateFix1OLD[2*k]=DateFix1[2*k];
                        DateFix1OLD[2*k+1]=DateFix1[2*k+1];
                    }                
                    Redraw[1]=false;                     
                }
            }
            else if (ScreenID==2) {
                ;                
            }
        }
        else {
            if (ScreenID==0) {
                // NA message
                if ((LoopNum+0)%5==0) LCD.Draw8x16(0,1*8,78,65,f);
            }                
            else if (ScreenID==1) {
                ;                
            }
            else if (ScreenID==2) {
                ;                
            }
        }


        // ***** LATITUDE *****
        // In ROW3 display latitude in ddddmm.mmmmmC format, replacing any leading 0's in degrees
        // and minutes by blanks.
        // check for validity by getting position of '.' which must be 4
        GOODdata = true;
        j=0; do {d=Latitude1[j]; j++;} while ((d!='.')&&(j<BSML)); // check '.' exists!
        if (j==BSML) {GOODdata=false;} else { j--; if (j!=4) {GOODdata=false;} }

        // display latitude if data valid else NA message
        if (GOODdata&&((LoopNum+1)%5==0)) {
            // Create characters for latitude
            // always j==4 ie. 2 digits in degrees but if 1st digit '0' then make it blank
            Latitude[0] = 32;
            tf = (int)Latitude1[0]; (tf==48) ? Latitude[1]=32 : Latitude[1]=tf;  // deal with any leading 0
            Latitude[2] = (int)Latitude1[1];
            Latitude[3] = 144; // degree char
            Latitude[4] = 32;
            tf = (int)Latitude1[2]; (tf==48) ? Latitude[5]=32 : Latitude[5]=tf; // deal with any leading 0
            k=4; do {Latitude[k+2]=(int)Latitude1[k-1]; k++;} while (Latitude1[k-3+SECA]!=0);
            Latitude[k++] = 39;//148; // seconds char
            Latitude[k++] = (int)Latitude2[0]; // seconds char
            Latitude[k] = 32;  // filler blank (so there is an even number of columns)

            if (ScreenID==0) {
                // display Latitude in 14 char row starting at column 1
                // by comparing Latitude with LatitudeOLD and only redrawing changed 2 char blocks
                m=0;
                if (!Redraw[2]) {
                    for (k=0; k<7; k++) {
                        if ((Latitude[2*k]!=LatitudeOLD[2*k])||(Latitude[2*k+1]!=LatitudeOLD[2*k+1])) {
                            LCD.Draw8x16(k,3*8,Latitude[2*k],Latitude[2*k+1],f);
                            m++;
                        }
                        // save Latitude to LatitudeOLD for comparison at next redraw
                        LatitudeOLD[2*k]=Latitude[2*k];
                        LatitudeOLD[2*k+1]=Latitude[2*k+1];
                    }
                }                                        
                else {
                    for (k=0; k<7; k++) {
                        LCD.Draw8x16(k,3*8,Latitude[2*k],Latitude[2*k+1],f);
                        // save Latitude to LatitudeOLD for comparison at next redraw
                        LatitudeOLD[2*k]=Latitude[2*k];
                        LatitudeOLD[2*k+1]=Latitude[2*k+1];
                    }
                    Redraw[2]=false;                     
                }
            }
            else if (ScreenID==1) {
                ;                
            }
            else if (ScreenID==2) {
                ;                
            }
        }
        else {
            if (ScreenID==0) {
                // NA message
                if ((LoopNum+1)%5==0) LCD.Draw8x16(0,3*8,78,65,f);
            }                
            else if (ScreenID==1) {
                ;                
            }
            else if (ScreenID==2) {
                ;                
            }
        }


        // ***** LONGTITUDE *****
        // In ROW4 display longtitude in dddddmm.mmmmmC format, replacing any leading 0's in degrees
        // and minutes by blanks.
        // check for validity by getting position of '.' which must be 5
        GOODdata = true;
        j=0; do {d=Longtitude1[j]; j++;} while ((d!='.')&&(j<BSML)); // check '.' exists!
        if (j==BSML) {GOODdata=false;} else { j--; if (j!=5) {GOODdata=false;} }

        // display longtitude if data valid else NA message
        if (GOODdata&&((LoopNum+2)%5==0)) {
            // Create characters for longtitude
            // always j==5 ie. 3 digits in degrees but if first one or two digits '0' then make them blank
            tf = (int)Longtitude1[0]; (tf==48) ? Longtitude[0]=32 : Longtitude[0]=tf;  // deal with 1st leading 0
            tf1 = (int)Longtitude1[1]; ((tf==48)&&(tf1==48)) ? Longtitude[1]=32 : Longtitude[1]=tf1;  // deal with 2nd leading 0
            Longtitude[2] = (int)Longtitude1[2];
            Longtitude[3] = 144; // degree char
            Longtitude[4] = 32;
            tf = (int)Longtitude1[3]; (tf==48) ? Longtitude[5]=32 : Longtitude[5]=tf; // deal with any leading 0
            k=5; do {Longtitude[k+1]=(int)Longtitude1[k-1]; k++;} while (Longtitude1[k-2+SECA]!=0);
            Longtitude[k++] = 39;//148; // seconds char
            Longtitude[k++] = (int)Longtitude2[0]; // seconds char
            Longtitude[k] = 32; // filler blank (so there is an even number of columns)

            if (ScreenID==0) {
                // display Longtitude in 14 char row starting at column 1
                // by comparing Longtitude with LongtitudeOLD and only redrawing changed 2 char blocks
                if (!Redraw[3]) {
                    m=0;
                    for (k=0; k<7; k++) {
                        if ((Longtitude[2*k]!=LongtitudeOLD[2*k])||(Longtitude[2*k+1]!=LongtitudeOLD[2*k+1])) {
                            LCD.Draw8x16(k,4*8,Longtitude[2*k],Longtitude[2*k+1],f);
                            m++;
                        }
                        // save Longtitude to LongtitudeOLD for comparison at next redraw
                        LongtitudeOLD[2*k]=Longtitude[2*k];
                        LongtitudeOLD[2*k+1]=Longtitude[2*k+1];
                    }                
                }
                else {
                    for (k=0; k<7; k++) {
                        LCD.Draw8x16(k,4*8,Longtitude[2*k],Longtitude[2*k+1],f);
                        // save Longtitude to LongtitudeOLD for comparison at next redraw
                        LongtitudeOLD[2*k]=Longtitude[2*k];
                        LongtitudeOLD[2*k+1]=Longtitude[2*k+1];
                    }                
                    Redraw[3]=false;                     
                }
            }
            else if (ScreenID==1) {
                ;                
            }
            else if (ScreenID==2) {
                ;                
            }
        }
        else {
            if (ScreenID==0) {
                // NA message
                if ((LoopNum+2)%5==0) LCD.Draw8x16(0,4*8,78,65,f);
            }                
            else if (ScreenID==1) {
                ;                
            }
            else if (ScreenID==2) {
                ;                
            }
        }
    

        // ***** SPEED *****
        // In ROW5 display speed over ground (SOG) in km/h
        // check for validity by trying to convert null-terminated char array SpeedN to float
        GOODdata = true;
        float fSpeedN = atof(SpeedN); if (fSpeedN==0.0) {GOODdata=false;}
        
        // display SOG if data valid else NA message
        if (GOODdata) {
            
            if (ScreenID==0) {
                // Create characters for SOG in km/h
                float fSpeedK = fSpeedN*1.852; // convert from knots to km/h
                dtostrf(fSpeedK,5,1,SpeedK);
                for (k=0; k<5; k++) {SpeedK2[k]=SpeedK[k];}; SpeedK2[5] = ' ';
                
                // display SOG in 6 char row starting at column 3
                // by comparing SpeedK2 with SpeedK2OLD and only redrawing changed 2 char blocks
                if (!Redraw[4]) {
                    int m=0;
                    for (k=0; k<3; k++) {
                        if ((SpeedK2[2*k]!=SpeedK2OLD[2*k])||(SpeedK2[2*k+1]!=SpeedK2OLD[2*k+1])) {
                            LCD.Draw8x16(k+2,6*8,SpeedK2[2*k],SpeedK2[2*k+1],f);
                            m++;
                        }
                        // save SpeedK2 to SpeedK2OLD for comparison at next redraw
                        SpeedK2OLD[2*k]=SpeedK2[2*k];
                        SpeedK2OLD[2*k+1]=SpeedK2[2*k+1];
                    }                
                }
                else { // do full redraw and set redraw flag to false.
                    for (k=0; k<3; k++) {
                        LCD.Draw8x16(k+2,6*8,SpeedK2[2*k],SpeedK2[2*k+1],f);
                        // save SpeedK2 to SpeedK2OLD for comparison at next redraw
                        SpeedK2OLD[2*k]=SpeedK2[2*k];
                        SpeedK2OLD[2*k+1]=SpeedK2[2*k+1];
                    }                
                    Redraw[4]=false;                     
                }
            }                
            else if (ScreenID==1) {
                ;                
            }
            else if (ScreenID==2) {
                // Create characters for SOG in km/h
                float fSpeedK = fSpeedN*1.852; // convert from knots to km/h
                dtostrf(fSpeedK,3,0,SpeedK);
                Serial.print("fSPEED="); Serial.print(fSpeedK); Serial.print("=\n");
                Serial.print("sSPEED="); Serial.print(SpeedK); Serial.print("=\n");
                for (k=0; k<3; k++) {Serial.print(SpeedK[k]); Serial.print("-"); }
                Serial.print("\n");
                
                // display SOG by using SpeedK and always a full redraw (optimization glitched!)
                int cn,sc;
                for (k=0; k<3; k++) {
                    sc = SpeedK[k];
                    if ((48<=sc)&&(sc<=57)) {
                        cn = (int)sc-48;                             
                    }
                    else { // if sc==32
                        cn = 10;                        
                    }
                    PrintBigChar(cn, k, Speed, BigFont);
                }
                LCD.Draw32x96(1,16,Speed);
            }
        }            
        else {
            if (ScreenID==0) {
                // NA message
                SpeedK2OLD[0]=78; SpeedK2OLD[1]=65;
                SpeedK2OLD[2]=SCHR; SpeedK2OLD[3]=SCHR;
                SpeedK2OLD[4]=SCHR; SpeedK2OLD[5]=SCHR;
                for (k=0; k<3; k++) LCD.Draw8x16(k+2,6*8,SpeedK2OLD[2*k],SpeedK2OLD[2*k+1],f);
            }                
            else if (ScreenID==1) {
                ;                
            }
            else if (ScreenID==2) {
                ;                
            }
        }


        // ***** ALTITUDE *****
        // In ROW6 display Altitude above mean sea level in metres
        // check for validity by trying to convert null-terminated char array AltitudeM to float
        GOODdata = true;
        float fAltitudeM = atof(AltitudeM);
        if (fAltitudeM==0.0) {GOODdata=false;}
        
        // display ASL if data valid else NA message
        if (GOODdata&&((LoopNum+4)%5==0)) {
            // Create characters for ASL in metres
            dtostrf(fAltitudeM,7,1,AltitudeM2);
            for (k=0; k<7; k++) {AltitudeM3[k]=AltitudeM2[k];}; AltitudeM3[7] = ' ';
            
            if (ScreenID==0) {
                // display ASL in 8 char row starting at column 3
                // by comparing AltitudeM3 with AltitudeM3OLD and only redrawing changed 2 char blocks
                if (!Redraw[5]) {
                    m=0;
                    for (k=0; k<4; k++) {
                        if ((AltitudeM3[2*k]!=AltitudeM3OLD[2*k])||(AltitudeM3[2*k+1]!=AltitudeM3OLD[2*k+1])) {
                            LCD.Draw8x16(k+2,7*8,AltitudeM3[2*k],AltitudeM3[2*k+1],f);
                            m++;
                        }
                        // save AltitudeM3 to AltitudeM3OLD for comparison at next redraw
                        AltitudeM3OLD[2*k]=AltitudeM3[2*k];
                        AltitudeM3OLD[2*k+1]=AltitudeM3[2*k+1];
                    }                
                }
                else {
                    for (k=0; k<4; k++) {
                        LCD.Draw8x16(k+2,7*8,AltitudeM3[2*k],AltitudeM3[2*k+1],f);
                        // save AltitudeM3 to AltitudeM3OLD for comparison at next redraw
                        AltitudeM3OLD[2*k]=AltitudeM3[2*k];
                        AltitudeM3OLD[2*k+1]=AltitudeM3[2*k+1];
                    }                
                    Redraw[5]=false;                                                             
                }
            }                
            else if (ScreenID==1) {
                ;                
            }
            else if (ScreenID==2) {
                ;                
            }
        }            
        else {
            if (ScreenID==0) {
                // NA message
                if ((LoopNum+4)%5==0) {
                    AltitudeM3OLD[0]=78; AltitudeM3OLD[1]=65;
                    AltitudeM3OLD[2]=SCHR; AltitudeM3OLD[3]=SCHR;
                    AltitudeM3OLD[4]=SCHR; AltitudeM3OLD[5]=SCHR;
                    AltitudeM3OLD[6]=SCHR; AltitudeM3OLD[7]=SCHR;
                    for (k=0; k<4; k++) LCD.Draw8x16(k+2,7*8,AltitudeM3OLD[2*k],AltitudeM3OLD[2*k+1],f);
                }
            }                
            else if (ScreenID==1) {
                ;                
            }
            else if (ScreenID==2) {
                ;                
            }
        }
    }
    
Serial.print(" -----\n");
}

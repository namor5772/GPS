// IN GITHUB
C:\Users\roman\Documents\GitHub\GPS\Code\MyST7920

#include "ST7920.h"


// Constructor
ST7920::ST7920(int dt) {
  this->_delaytime = dt;
} 


void ST7920::InitDriverPins(int clock,int latch,int data) {
  _clockPin = clock; // SCLK - System Clock, Serial Clock - Nano D13
  _latchPin = latch; // SS - Slave Select - Nano D10 (CS - chip select)
  _dataPin = data;   // MOSI (SID) - Master Out Slave In, Serial Data - Nano D11 
}


void ST7920::Init() {
  pinMode(_latchPin, OUTPUT); digitalWrite(_latchPin, LOW);
  pinMode(_clockPin, OUTPUT); digitalWrite(_clockPin, LOW);    
  pinMode(_dataPin, OUTPUT);  digitalWrite(_dataPin, LOW);
  delayMicroseconds(120);    

  WriteCommand(0x30); // b0011000 Function Set: 8-bt interface, Basic instructions   
  WriteCommand(0x0c); // b00001100 Display Control: Display ON, Cursor OFF, Char Blink OFF       
  WriteCommand(0x01); // b00000001 Display Clear      
  WriteCommand(0x06); // b00000110 Entry Mode Set: Increment mode ON, Entire Shift OFF      
  WriteCommand(0x36); // b00110100 Function Set: 8-bt interface, (DB2) RE=1 extended instruction set. (DB10) G=0 graphic display OFF.
}


// the guts of bit-bashed SPI with MSB first
void ST7920::WriteByte(int dat) {
    digitalWrite(_latchPin, HIGH);
    delayMicroseconds(_delaytime); // 31 is lowest without glitching sometimes   

    for (uint8_t i = 0; i < 8; i++) {
        digitalWrite(_dataPin, !!(dat & (1 << (7 - i))));
        digitalWrite(_clockPin, HIGH);
        digitalWrite(_clockPin, LOW);
    }
    digitalWrite(_latchPin, LOW);
}

void ST7920::WriteCommand(int CMD) {
    int H_data,L_data;
    H_data = CMD; 
    H_data &= 0xf0;  // =b D7,D6,D5,D4,0000             
    L_data = CMD;             
    L_data &= 0x0f;  // =b 0000,D3,D2,D1,D0         
    L_data <<= 4;    // =b D3,D2,D1,D0,0000          
    WriteByte(0xf8); // =b11111,R/W,RS,0 (R/W=0, RS=0)          
    WriteByte(H_data); 
    WriteByte(L_data); 
}


void ST7920::WriteData(int DAT) {
    int H_data,L_data;
    
    H_data = DAT;
    H_data &= 0xf0;  // =b D7,D6,D5,D4,0000                        
    L_data = DAT;             
    L_data &= 0x0f;  // =b 0000,D3,D2,D1,D0                  
    L_data <<= 4;    // =b D3,D2,D1,D0,0000                   
    
    WriteByte(0xfa); // =b11111,R/W,RS,0 (R/W=0, RS=1)                   
    WriteByte(H_data);
    WriteByte(L_data);
}


void ST7920::Clear(void) {
    int ygroup,x,y,i;
    for(ygroup=0;ygroup<64;ygroup++) {                           
        if (ygroup<32) { x=0x80; y=ygroup+0x80; }
        else { x=0x88; y=ygroup-32+0x80; }
        WriteCommand(y); // set vertical address first b1000,AC3,AC2,AC1,AC0           
        WriteCommand(x); // horizontal next b10,AC5,AC4,AC3,AC2,AC1,AC0 
        for(i=0;i<16;i++) { WriteData(0x00); }
    }
}


void ST7920::DrawFullScreen(const unsigned char *p) {
    int ygroup,x,y,i,tmp;
    for(ygroup=0;ygroup<64;ygroup++) {                           
        if (ygroup<32) { x=0x80; y=ygroup+0x80; }
        else { x=0x88; y=ygroup-32+0x80; }
        WriteCommand(y); // set vertical address first b1000,AC3,AC2,AC1,AC0           
        WriteCommand(x); // horizontal next b10,AC5,AC4,AC3,AC2,AC1,AC0 
        tmp=ygroup*16;
        for(i=0;i<16;i++) { WriteData(pgm_read_byte_far(p+tmp++)); }
    }
}


// x0 = 0,1,2. (actual column x0*16)
// y0 = 0,..,32 (actual bit rows)
// draws 32x96 from p.
void ST7920::Draw32x96(int x0, int y0, unsigned char *p) {
    int ygroup,x,y,i;
    unsigned int tmp;
      
    for(ygroup=y0;ygroup<(32+y0);ygroup++) {                           
        if (ygroup<32) { 
            x=0x80+x0;
            y=ygroup+0x80;
        }
        else {
            x=0x88+x0;
            y=ygroup-32+0x80;
        }
        WriteCommand(y); // set vertical address first b1000,AC3,AC2,AC1,AC0           
        WriteCommand(x); // horizontal next b10,AC5,AC4,AC3,AC2,AC1,AC0     
        
        tmp=(ygroup-y0)*12;
        for(i=0;i<12;i++) {
            WriteData(p[tmp++]);
        }
    }
}


// x0 = 0,..,7 (actual column x0*16)
// y0 = 0,..,63 (actual bit rows)
// draws 8x16 bit map with top left hand bit position (y0,x0*16) till (y0+7,x0*16+15)
// draws char with ascii code C in first 8x8 block and D in block on its right.
void ST7920::Draw8x16(int x0, int y0, int C, int D, const unsigned char *p) {
    int ygroup,x,y,i,tmpC=C*8,tmpD=D*8;
    for(ygroup=y0;ygroup<y0+8;ygroup++) {                           
        if (ygroup<32) { x=0x80+x0; y=ygroup+0x80; }
        else { x=0x88+x0; y=ygroup-32+0x80; }
        WriteCommand(y);           
        WriteCommand(x);     
        WriteData(pgm_read_byte_far(p+tmpC++));
        WriteData(pgm_read_byte_far(p+tmpD++));
    }
}


// x0 = 0,..,7 (actual column x0*16)
// y0 = 0,..,63 (actual bit rows)
// draws 24x16 bit map with top left hand bit position (y0,x0*16) till (y0+23,x0*16+15)
// draws char C='0,...,9,:' as integers 0,..,10 with font data in 24x176 char array f
void ST7920::Draw24x16(int x0, int y0, int C, unsigned char *f) {
    int ygroup, x, y;
    unsigned int p;
      
    for(ygroup=y0;ygroup<(y0+24);ygroup++) {                           
        if (ygroup<32) { 
            x=0x80+x0;
            y=ygroup+0x80;
        }
        else {
            x=0x88+x0;
            y=ygroup-32+0x80;
        }
        WriteCommand(y); // set vertical address first b1000,AC3,AC2,AC1,AC0           
        WriteCommand(x); // horizontal next b10,AC5,AC4,AC3,AC2,AC1,AC0     
        
        p=(ygroup-y0)*22+C*2; // find correct position in f
        WriteData(f[p++]);
        WriteData(f[p]);
    }
}

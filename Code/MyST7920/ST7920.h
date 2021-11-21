// IN GITHUB
// C:\Users\roman\Documents\GitHub\GPS\Code\MyST7920

#ifndef LCD_ST7920_h
#define LCD_ST7920_h
#include <Arduino.h>

class ST7920 {
    typedef unsigned char uchar;

    public:

    ST7920(int dt); // Constructor

    void InitDriverPins(int clock,int latch,int data);
    void Init(void);

    void WriteByte(int dat);
    void WriteCommand(int CMD);
    void WriteData(int DAT);

    void Clear(void);
    void DrawFullScreen(const unsigned char *p);
    void Draw32x96(int x0, int y0, unsigned char *p);
    void Draw32x96_(int x0, int y0, unsigned char *p, unsigned char *pold, bool fFull, bool fSave);
    void Draw8x16(int x0, int y0, int C, int D, const unsigned char *p);
    void Draw24x16(int x0, int y0, int C, unsigned char *f);


    private:        

    int _delaytime;

    int _latchPin ; 
    int _clockPin ;  
    int _dataPin ;  
};

//extern LCD_ST7920 LCD;    
#endif

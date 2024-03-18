#include <Arduino.h>
#include <MCUFRIEND_kbv.h>

#define USE_XPT2046   0
#define USE_LOCAL_KBV 1

MCUFRIEND_kbv tft;

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

#define RGB(r, g, b) (((r&0xF8)<<8)|((g&0xFC)<<3)|(b>>3))

#define GREY      RGB(127, 127, 127)
#define DARKGREY  RGB(64, 64, 64)
#define TURQUOISE RGB(0, 128, 128)
#define PINK      RGB(255, 128, 192)
#define OLIVE     RGB(128, 128, 0)
#define PURPLE    RGB(128, 0, 128)
#define AZURE     RGB(0, 128, 255)
#define ARDUINO_TURQUOISE RGB(0,151,156)

#define LANDSCAPE 1

struct RGB_Model{
    uint8_t red = 255;
    uint8_t green = 255;
    uint8_t blue = 255;
    uint8_t PointSize = 1;
};

struct RGB_Model tftColorTable;

// MCUFRIEND UNO shield shares pins with the TFT.
#if defined(ESP32)
int XP = 27, YP = 4, XM = 15, YM = 14;  //most common configuration
#else
//int XP = 6, YP = A1, XM = A2, YM = 7;  //most common configuration
int XP = 8, YP = A3, XM = A2, YM = 9;  //next common configuration
//int XP=PB7,XM=PA6,YP=PA7,YM=PB6; //BLUEPILL must have Analog for YP, XM
#endif
#if USE_LOCAL_KBV
#include "TouchScreen_kbv.h"         //my hacked version
#define TouchScreen TouchScreen_kbv
#define TSPoint     TSPoint_kbv
#else
#include <TouchScreen.h>         //Adafruit Library
#endif
TouchScreen ts(XP, YP, XM, YM, 300);   //re-initialised after diagnose
TSPoint tp;

/**** EXTRA FUNCTIONS  */
struct textTFT {
    uint16_t x;
    uint16_t y;
    uint16_t fcolor = BLACK;
    uint16_t bcolor = WHITE;
    uint8_t textSize = 1;
    String Text;
};

void setUpText( struct textTFT t )
{
    tft.setCursor(t.x, t.y);
    tft.setTextColor( t.fcolor , t.bcolor );
    tft.setTextSize( t.textSize );
    tft.println(t.Text);
}
/**** END  */

/**** SCREEN FUNCTIONS  */

#define BAR_WIDTH tft.width()
#define BAR_HEIGHT 40
#define BAR_POSX 0
#define BAR_POSY 280

#define LABEL_SIZE 30
#define LABEL_POSY 290
#define LABEL_FIRST_POSX 10
#define LABEL_SPACE_BTW 35
#define LABEL_PENCIL_WIDTH 110

#define SPACE_BTW(x) ( LABEL_FIRST_POSX + x*LABEL_SPACE_BTW )

void homeScreen()
{
    // Menu Bar
    tft.fillRect( BAR_POSX , BAR_POSY , BAR_WIDTH, BAR_HEIGHT, GREY );
    // RGB Colors
    tft.fillRect( SPACE_BTW(0) , LABEL_POSY , LABEL_SIZE, LABEL_SIZE, RGB( tftColorTable.red, 0,0 ) );
    tft.fillRect( SPACE_BTW(1) , LABEL_POSY , LABEL_SIZE, LABEL_SIZE, RGB( 0, tftColorTable.green,0 ) );
    tft.fillRect( SPACE_BTW(2) , LABEL_POSY , LABEL_SIZE, LABEL_SIZE, RGB( 0, 0,tftColorTable.blue ) );
    tft.fillRect( SPACE_BTW(3) , LABEL_POSY , 2*LABEL_SIZE, LABEL_SIZE, RGB( tftColorTable.red, tftColorTable.green ,tftColorTable.blue ) );
    // PENCIL
    tft.fillRect( SPACE_BTW(4) + LABEL_SIZE , LABEL_POSY , LABEL_PENCIL_WIDTH, LABEL_SIZE, WHITE );
    struct textTFT t;
    t.x = 190;
    t.y= 295;
    t.fcolor= BLACK;
    t.bcolor= WHITE;
    t.textSize = 3;
    t.Text = "PS: " + String(tftColorTable.PointSize);
    setUpText( t );
}

#define LOGO_POS_X 45
#define LOGO_POS_Y 165

#define LOGOTEXT_POS_X 85
#define LOGOTEXT_POS_Y 285

#define LOGO_SIZE_TEXT 3

void logoScreen()
{
    struct textTFT t;
    t.x = LOGO_POS_X;
    t.y= LOGO_POS_Y;
    t.fcolor= ARDUINO_TURQUOISE;
    t.bcolor = WHITE ;
    t.textSize = LOGO_SIZE_TEXT ;
    t.Text= "Here's the Arduino Logo";
    setUpText( t );
    // Set the Logo
    t.x = LOGOTEXT_POS_X;
    t.y= LOGOTEXT_POS_Y;
    t.Text = "TFT Graphic Table";
    setUpText( t );
}
/**** END  */

char *Aval(int pin)
{
    static char buf[2][10], cnt;
    cnt = !cnt;
#if defined(ESP32)
    sprintf(buf[cnt], "%d", pin);
#else
    sprintf(buf[cnt], "A%d", pin - A0);
#endif
    return buf[cnt];
}

void showpins(int A, int D, int value, const char *msg)
{
    char buf[40];
    sprintf(buf, "%s (%s, D%d) = %d", msg, Aval(A), D, value);
    Serial.println(buf);
}


void readResistiveTouch(void)
{
    tp = ts.getPoint();
    pinMode(YP, OUTPUT);      //restore shared pins
    pinMode(XM, OUTPUT);
    //digitalWrite(YP, HIGH);  //because TFT control pins
    //digitalWrite(XM, HIGH);
    //Serial.println("tp.x=" + String(tp.x) + ", tp.y=" + String(tp.y) + ", tp.z =" + String(tp.z));
}

bool ISPRESSED(void)
{
    // .kbv this was too sensitive !!
    // now touch has to be stable for 50ms
    int count = 0;
    bool state, oldstate;
    while (count < 10) {
        readResistiveTouch();
        state = tp.z > 200;     //ADJUST THIS VALUE TO SUIT YOUR SCREEN e.g. 20 ... 250
        if (state == oldstate) count++;
        else count = 0;
        oldstate = state;
        delay(5);
    }
    if( oldstate ){
      Serial.println("tp.x=" + String(tp.x) + ", tp.y=" + String(tp.y) + ", tp.z =" + String(tp.z));
    }
    return oldstate;
}

#if USE_XPT2046 == 0
bool diagnose_pins()
{
    uint8_t i, j, Apins[2], Dpins[2], found = 0;
    uint16_t value, Values[2];

    Serial.println(F("Making all control and bus pins INPUT_PULLUP"));
    Serial.println(F("Typical 30k Analog pullup with corresponding pin"));
    Serial.println(F("would read low when digital is written LOW"));
    Serial.println(F("e.g. reads ~25 for 300R X direction"));
    Serial.println(F("e.g. reads ~30 for 500R Y direction"));
    Serial.println(F(""));

    for (i = A0; i < A5; i++) pinMode(i, INPUT_PULLUP);
    for (i = 2; i < 10; i++) pinMode(i, INPUT_PULLUP);
    for (i = A0; i < A4; i++) {
        pinMode(i, INPUT_PULLUP);
        for (j = 5; j < 10; j++) {
            pinMode(j, OUTPUT);
            digitalWrite(j, LOW);
            value = analogRead(i);               // ignore first reading
            value = analogRead(i);
            if (value < 100 && value > 0) {
                showpins(i, j, value, "Testing :");
                if (found < 2) {
                    Apins[found] = i;
                    Dpins[found] = j;
                    Values[found] = value;
                }
                found++;
            }
            pinMode(j, INPUT_PULLUP);
        }
        pinMode(i, INPUT_PULLUP);
    }
    if (found == 2) {
        int idx = Values[0] < Values[1];
                Serial.println(F("Diagnosing as:-"));
                for (i = 0; i < 2; i++) {
                    showpins(Apins[i], Dpins[i], Values[i],
                             (Values[i] < Values[!i]) ? "XM,XP: " : "YP,YM: ");
                }
        /**/
        XM = Apins[!idx]; XP = Dpins[!idx]; YP = Apins[idx]; YM = Dpins[idx];
        ts = TouchScreen(XP, YP, XM, YM, 300);    //re-initialise with pins
        return true;                              //success
    }
    if (found == 0) Serial.println(F("MISSING TOUCHSCREEN"));
    //else Serial.println(F("BROKEN TOUCHSCREEN"));
    return false;
}
#endif

void setTouchScreenMode(uint16_t identifier)
{
  bool ret = true;
  #if USE_XPT2046 || defined(__arm__) || defined(ESP32)
      Serial.println(F("Not possible to diagnose Touch pins on ARM or ESP32"));
  #else
      ret = diagnose_pins();  //destroys TFT pin modes
      tft.begin(identifier);
      tft.fillScreen(WHITE);
      tft.setRotation(LANDSCAPE);
  #endif
}

void setup()
{
    // put your setup code here, to run once:
    Serial.begin(9600);
    tft.reset();
    uint16_t identifier = tft.readID();
    Serial.print("ID = 0x");
    Serial.println(identifier, HEX);
    if (identifier == 0xEFEF) identifier = 0x9486;
    tft.begin(identifier);
    tft.fillScreen(WHITE);
    tft.setRotation(LANDSCAPE);
    //setTouchScreenMode(identifier);
    // Set Up Logo Animation
    //logoScreen();
    //delay(3000);
    // White Screen
    //tft.fillScreen(WHITE);
    //delay(500);
    // Show Home Screen
    homeScreen();
}

void loop()
{
  while (ISPRESSED() == false) {}
  while (ISPRESSED() == true) {}
}

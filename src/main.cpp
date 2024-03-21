#include <Arduino.h>
#include <MCUFRIEND_kbv.h>
#include <Adafruit_GFX.h>

#define PORTRAIT  0
#define LANDSCAPE 1
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

#define DARKRED   RGB(127, 0, 0)
#define GREY      RGB(127, 127, 127)
#define DARKGREY  RGB(64, 64, 64)
#define TURQUOISE RGB(0, 128, 128)
#define PINK      RGB(255, 128, 192)
#define OLIVE     RGB(128, 128, 0)
#define PURPLE    RGB(128, 0, 128)
#define AZURE     RGB(0, 128, 255)
#define ARDUINO_TURQUOISE RGB(0,151,156)

#define LANDSCAPE 1

uint8_t SCREEN = 0;

struct pencil_model{
  int x;
  int y;
  int z;
  uint8_t red = 0;
  uint8_t green = 0;
  uint8_t blue = 0;
  uint8_t PointSize = 1;
};

struct pencil_model tftColorTable;

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

//const int TS_LEFT=99,TS_RT=948,TS_TOP=147,TS_BOT=643;// Data obtained by calibration
const int TS_LEFT=94,TS_RT=951,TS_TOP=158,TS_BOT=656;
/**** EXTRA FUNCTIONS  */

void setTFTpoints( struct pencil_model *p , struct TSPoint tp)
{
  p->x = map(tp.y, TS_RT, TS_LEFT, 0, 480);
  p->y = map(tp.x, TS_BOT, TS_TOP, 0, 320);
  p->z = tp.z;
}

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


/************* END  **/

/***** Buttons */

Adafruit_GFX_Button button_refresh, button_ok;
bool button_refresh_pressed = false;

/****** END */

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

bool verifyRedColor(){
    return ( tftColorTable.y >= 285 && tftColorTable.x >= SPACE_BTW(0) && tftColorTable.x <= SPACE_BTW(0) + LABEL_SIZE );
}

bool verifyGreenColor(){
    return ( tftColorTable.y >= 285 && tftColorTable.x >= SPACE_BTW(1) && tftColorTable.x <= SPACE_BTW(1) + LABEL_SIZE );
}

bool verifyBlueColor(){
    return ( tftColorTable.y >= 285 && tftColorTable.x >= SPACE_BTW(2) && tftColorTable.x <= SPACE_BTW(2) + LABEL_SIZE );
}

bool verifyPecilSize(){
    return ( tftColorTable.y >= 285 && tftColorTable.x >= SPACE_BTW(4) + LABEL_SIZE && tftColorTable.x <= SPACE_BTW(4) + LABEL_SIZE + LABEL_PENCIL_WIDTH );
}

void homeScreen()
{
  SCREEN = 2;
    // Menu Bar
    tft.fillRect( BAR_POSX , BAR_POSY , BAR_WIDTH, BAR_HEIGHT, GREY );
    tft.fillRect( 440 , 0 , 40, tft.height(), GREY );
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
    button_refresh.initButton(&tft,  340, 300, 80, 38, BLACK, GREEN, WHITE, "REF", 2);
    button_refresh.drawButton(false);
}

#define LOGO_POS_X 45
#define LOGO_POS_Y 165

#define LOGOTEXT_POS_X 85
#define LOGOTEXT_POS_Y 165

#define LOGO_SIZE_TEXT 3

void logoScreen()
{
  SCREEN = 1;
    struct textTFT t;
    //t.x = LOGO_POS_X;
    //t.y= LOGO_POS_Y;
    t.fcolor= ARDUINO_TURQUOISE;
    t.bcolor = WHITE ;
    t.textSize = LOGO_SIZE_TEXT ;
    //t.Text= "Here's the Arduino Logo";
    //setUpText( t );
    // Set the Logo
    t.x = LOGOTEXT_POS_X;
    t.y= LOGOTEXT_POS_Y;
    t.Text = "TFT Graphic Table";
    setUpText( t );
}
/**** END  */


/************* Touch Screen Module  **/

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
    //Serial.println(buf);
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
    setTFTpoints( &tftColorTable ,  tp );
    return oldstate;
}

#if USE_XPT2046 == 0
bool diagnose_pins()
{
    uint8_t i, j, Apins[2], Dpins[2], found = 0;
    uint16_t value, Values[2];

    //Serial.println(F("Making all control and bus pins INPUT_PULLUP"));
    //Serial.println(F("Typical 30k Analog pullup with corresponding pin"));
    //Serial.println(F("would read low when digital is written LOW"));
    //Serial.println(F("e.g. reads ~25 for 300R X direction"));
    //Serial.println(F("e.g. reads ~30 for 500R Y direction"));
    //Serial.println(F(""));

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
        /*        Serial.println(F("Diagnosing as:-"));
                for (i = 0; i < 2; i++) {
                    showpins(Apins[i], Dpins[i], Values[i],
                             (Values[i] < Values[!i]) ? "XM,XP: " : "YP,YM: ");
                }
        */
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
/************* END  **/

/************* Calibration Screen Module  **/
#define TITLE "TouchScreen.h GFX Calibration"
#define GRAY_READY BLUE      //finished cross-hair
#define GRAY_DONE RED      //finished cross-hair

#define TOUCH_ORIENTATION  PORTRAIT
#define SWAP(x, y) { int t = x; x = y; y = t; }

uint32_t cx, cy, cz;
uint32_t rx[8], ry[8];
int32_t clx, crx, cty, cby;
float px, py;
int dispx, dispy, text_y_center, swapxy;
uint32_t calx, caly, cals;

void centertitle(const char *s);

uint16_t readID(void) {
    uint16_t ID = tft.readID();
    if (ID == 0xD3D3) ID = 0x9486;
    return ID;
}

void bofe(char *buf)
{
    tft.println(buf);
    //Serial.println(buf);
}

void report()
{
    uint16_t TS_LEFT, TS_RT, TS_TOP, TS_BOT, TS_WID, TS_HT, TS_SWAP;
    int16_t tmp;
    char buf[60];
    centertitle(TITLE);

    tft.println(F("To use the new calibration"));
    tft.println(F("settings you must map the values"));
    tft.println(F("from Point p = ts.getPoint() e.g. "));
    tft.println(F("x = map(p.x, LEFT, RT, 0, tft.width());"));
    tft.println(F("y = map(p.y, TOP, BOT, 0, tft.height());"));
    tft.println(F("swap p.x and p.y if diff ORIENTATION"));

    //.kbv show human values
    TS_LEFT = (calx >> 14) & 0x3FFF;
    TS_RT   = (calx >>  0) & 0x3FFF;
    TS_TOP  = (caly >> 14) & 0x3FFF;
    TS_BOT  = (caly >>  0) & 0x3FFF;
    TS_WID  = ((cals >> 12) & 0x0FFF) + 1;
    TS_HT   = ((cals >>  0) & 0x0FFF) + 1;
    TS_SWAP = (cals >> 31);
    if (TOUCH_ORIENTATION == LANDSCAPE) { //always show PORTRAIT first
        tmp = TS_LEFT, TS_LEFT = TS_BOT, TS_BOT = TS_RT, TS_RT = TS_TOP, TS_TOP = tmp;
        tmp = TS_WID, TS_WID = TS_HT, TS_HT = tmp;
    }
    sprintf(buf, "\n\n*** COPY-PASTE from Serial Terminal:");
    bofe(buf);
    bool ts_landscape = (TOUCH_ORIENTATION == LANDSCAPE) ^ swapxy;
#if (USE_XPT2046)
    sprintf(buf, "const int TS_LANDSCAPE=%d; //%s", ts_landscape, TITLE);
    bofe(buf);
#else
    if (ts_landscape) {
        SWAP(XM, YP);
        SWAP(XP, YM);
        SWAP(TS_LEFT, TS_RT);
        SWAP(TS_TOP, TS_BOT);
    }
    sprintf(buf, "const int XP=%d,XM=%s,YP=%s,YM=%d; //%dx%d ID=0x%04X",
            XP, Aval(XM), Aval(YP), YM, TS_WID, TS_HT, readID());
    Serial.println(buf);
    sprintf(buf, "\nTouch Pin Wiring XP=%d XM=%s YP=%s YM=%d",
            XP, Aval(XM), Aval(YP), YM);
    tft.println(buf);
#endif
    sprintf(buf, "const int TS_LEFT=%d,TS_RT=%d,TS_TOP=%d,TS_BOT=%d;",
            TS_LEFT, TS_RT, TS_TOP, TS_BOT);
    Serial.println(buf);

#if !defined(ARDUINO_AVR_LEONARDO)
    for (int orient = 0; orient < 2; orient++) {
        sprintf(buf, "\n%s CALIBRATION     %d x %d",
                orient ? "LANDSCAPE" : "PORTRAIT ", TS_WID, TS_HT);
        bofe(buf);
        sprintf(buf, "x = map(p.%s, LEFT=%d, RT=%d, 0, %d)",
                orient ? "y" : "x", TS_LEFT, TS_RT, TS_WID);
        bofe(buf);
        sprintf(buf, "y = map(p.%s, TOP=%d, BOT=%d, 0, %d)",
                orient ? "x" : "y", TS_TOP, TS_BOT, TS_HT);
        bofe(buf);
        tmp = TS_LEFT, TS_LEFT = TS_TOP, TS_TOP = TS_RT, TS_RT = TS_BOT, TS_BOT = tmp;
        tmp = TS_WID, TS_WID = TS_HT, TS_HT = tmp;
    }

    int16_t x_range = TS_LEFT - TS_RT, y_range = TS_TOP - TS_BOT;
    if (abs(x_range) > 500 && abs(y_range) > 650) //LANDSCAPE
        return;
    sprintf(buf, "\n*** UNUSUAL CALIBRATION RANGES %d %d", x_range, y_range);
    bofe(buf);
#endif
}

void fail()
{
    centertitle("Touch Calibration FAILED");

    tft.println(F("Unable to read the position"));
    tft.println(F("of the press. This is a"));
    tft.println(F("hardware issue and can not"));
    tft.println(F("be corrected in software."));
    tft.println(F("check XP, XM pins with a multimeter"));
    tft.println(F("check YP, YM pins with a multimeter"));
    tft.println(F("should be about 300 ohms"));

    while (true) {};
}


void drawCrossHair(int x, int y, uint16_t color)
{
    tft.drawRect(x - 10, y - 10, 20, 20, color);
    tft.drawLine(x - 5, y, x + 5, y, color);
    tft.drawLine(x, y - 5, x, y + 5, color);
}

void centerprint(const char *s, int y)
{
    int len = strlen(s) * 6;
    tft.setTextColor(WHITE, RED);
    tft.setCursor((dispx - len) / 2, y);
    tft.print(s);
}


void centertitle(const char *s)
{
    tft.fillScreen(BLACK);
    tft.fillRect(0, 0, dispx, 14, RED);
    tft.fillRect(0, 14, dispx, 1, WHITE);
    centerprint(s, 1);
    tft.setCursor(0, 30);
    tft.setTextColor(WHITE, BLACK);
}


void startup()
{
    dispx = tft.width();
    dispy = tft.height();

    text_y_center = (dispy / 2) - 6;

    centertitle(TITLE);

    tft.println(F("#define NUMSAMPLES 3 in Library\n"));
    tft.println(F("Use a stylus or something"));
    tft.println(F("similar to touch as close"));
    tft.println(F("to the center of the WHITE"));
    tft.println(F("crosshair.  Keep holding"));
    tft.println(F("until crosshair turns RED."));
    tft.println(F("Repeat for all crosshairs.\n"));
    tft.println(F("Report can be pasted from Serial\n"));
    tft.println(F("Touch screen to continue"));

    while (ISPRESSED() == false) {}
    while (ISPRESSED() == true) {}
    //    waitForTouch();
}

void readCoordinates()
{
    int iter = 5000;
    int failcount = 0;
    int cnt = 0;
    uint32_t tx = 0;
    uint32_t ty = 0;
    uint32_t tz = 0;
    bool OK = false;

    while (OK == false)
    {
        centerprint("*  PRESS  *", text_y_center);
        while (ISPRESSED() == false) {}
        centerprint("*  HOLD!  *", text_y_center);
        cnt = 0;
        iter = 400;
        do
        {
            readResistiveTouch();
            if (tp.z > 200)  //.kbv
            {
                tx += tp.x;
                ty += tp.y;
                tz += tp.z;
                cnt++;
            }
            else
                failcount++;
        } while ((cnt < iter) && (failcount < 10000));
        if (cnt >= iter)
        {
            OK = true;
        }
        else
        {
            tx = 0;
            ty = 0;
            tz = 0;
            cnt = 0;
        }
        if (failcount >= 10000)
            fail();
    }

    cx = tx / iter;
    cy = ty / iter;
    cz = tz / iter;
}


void calibrate(int x, int y, int i, String msg)
{
    drawCrossHair(x, y, GRAY_READY);
    readCoordinates();
    centerprint("* RELEASE *", text_y_center);
    drawCrossHair(x, y, GRAY_DONE);
    rx[i] = cx;
    ry[i] = cy;
    char buf[40];
    sprintf(buf, "\r\ncx=%ld cy=%ld cz=%ld %s", cx, cy, cz, msg.c_str());
    Serial.print(buf);
    while (ISPRESSED() == true) {}
}

void calibrationScreen()
{
  startup();

  int x, y, cnt, idx = 0;
  tft.fillScreen(BLACK);
  for (x = 10, cnt = 0; x < dispx; x += (dispx - 20) / 2) {
      for (y = 10; y < dispy; y += (dispy - 20) / 2) {
          if (++cnt != 5) drawCrossHair(x, y, GREY);
      }
  }
  centerprint("***********", text_y_center - 12);
  centerprint("***********", text_y_center + 12);
  for (x = 10, cnt = 0; x < dispx; x += (dispx - 20) / 2) {
      for (y = 10; y < dispy; y += (dispy - 20) / 2) {
          if (++cnt != 5) calibrate(x, y, idx++, F(" X, Y, Pressure"));
      }
  }
  cals = (long(dispx - 1) << 12) + (dispy - 1);
  swapxy = rx[2] - rx[0];
  //else swapxy = ry[2] - ry[0];
  swapxy = (swapxy < -400 || swapxy > 400);
  if (swapxy != 0) {
      clx = (ry[0] + ry[1] + ry[2]); //rotate 90
      crx = (ry[5] + ry[6] + ry[7]);
      cty = (rx[0] + rx[3] + rx[5]);
      cby = (rx[2] + rx[4] + rx[7]);
  } else {
      clx = (rx[0] + rx[1] + rx[2]); //regular
      crx = (rx[5] + rx[6] + rx[7]);
      cty = (ry[0] + ry[3] + ry[5]);
      cby = (ry[2] + ry[4] + ry[7]);
  }
  clx /= 3;
  crx /= 3;
  cty /= 3;
  cby /= 3;
  px = float(crx - clx) / (dispx - 20);
  py = float(cby - cty) / (dispy - 20);
  //  px = 0;
  clx -= px * 10;
  crx += px * 10;
  cty -= py * 10;
  cby += py * 10;
  calx = (long(clx) << 14) + long(crx);
  caly = (long(cty) << 14) + long(cby);
  if (swapxy)
      cals |= (1L << 31);
  report();          // report results
  while (ISPRESSED() == false) {}
  while (ISPRESSED() == true) {}

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
    logoScreen();
    delay(3000);
    // White Screen
    tft.fillScreen(WHITE);
    delay(500);
    // Show Home Screen
    homeScreen();
    //calibrationScreen();
    //tft.fillScreen(WHITE);
    //homeScreen();
}

bool verifybar()
{
    bool ybar = tftColorTable.y >= 10 && tftColorTable.y <= 240;
    bool xbar = tftColorTable.x >= 445 && tftColorTable.y <= 475;
    return ybar && xbar;
    
}

void loop()
{
    static bool RedState, GreenState, BlueState, PencilState;
  while (ISPRESSED() == true) {
    if ( tftColorTable.y >= 3 && tftColorTable.y <= 275 && tftColorTable.x >= 3 && tftColorTable.x <= 435 && SCREEN == 2  ){
      uint16_t colorPoint = RGB( tftColorTable.red , tftColorTable.green , tftColorTable.blue );
      tft.drawCircle(tftColorTable.x, tftColorTable.y, tftColorTable.PointSize*2, colorPoint  );
      tft.fillCircle(tftColorTable.x, tftColorTable.y, tftColorTable.PointSize*2, colorPoint );
    } 
    else if ( tftColorTable.x >= 440 && tftColorTable.y < 280 && SCREEN == 2  )
    {
        if (RedState || BlueState || GreenState){
            if ( verifybar() ){
                int16_t set_color = 0;
                if (RedState) set_color= RED;
                if (BlueState) set_color= BLUE;
                if (GreenState) set_color= GREEN;
                
                int SecondRectangleYpos = tftColorTable.y;
                int SecondRectangleHeigth = SecondRectangleYpos - 10;

                int firstRectangleYpos = 240;
                int firstRectangleHeigth = 230 - SecondRectangleYpos;
                
                tft.fillRect( 450, firstRectangleYpos - firstRectangleHeigth, 20, firstRectangleHeigth, set_color );
                tft.fillRect( 450, 10, 20, SecondRectangleHeigth, BLACK );

                int newcolor = map(tftColorTable.y, 240, 10 , 0 , 255  );
                if(RedState) tftColorTable.red = newcolor;
                if(BlueState) tftColorTable.blue = newcolor;
                if(GreenState) tftColorTable.green = newcolor;

                // RGB Colors
                tft.fillRect( SPACE_BTW(0) , LABEL_POSY , LABEL_SIZE, LABEL_SIZE, RGB( tftColorTable.red, 0,0 ) );
                tft.fillRect( SPACE_BTW(1) , LABEL_POSY , LABEL_SIZE, LABEL_SIZE, RGB( 0, tftColorTable.green,0 ) );
                tft.fillRect( SPACE_BTW(2) , LABEL_POSY , LABEL_SIZE, LABEL_SIZE, RGB( 0, 0,tftColorTable.blue ) );
                tft.fillRect( SPACE_BTW(3) , LABEL_POSY , 2*LABEL_SIZE, LABEL_SIZE, RGB( tftColorTable.red, tftColorTable.green ,tftColorTable.blue ) );


            }
            bool down = tftColorTable.z > 200;
            button_ok.press(down && button_ok.contains(tftColorTable.x, tftColorTable.y));
            if (button_ok.justPressed()) {
              button_ok.drawButton(true);
              Serial.println("Pressed button ok");
              while( !button_ok.justReleased()){
                ISPRESSED();
                down = tftColorTable.z > 200;
                button_ok.press(down && button_ok.contains(tftColorTable.x, tftColorTable.y));
              }
              button_ok.drawButton();
                RedState = false;
                GreenState = false;
                BlueState = false;
                PencilState = false;
              tft.fillRect( 440 , 0 , 40, tft.height(), GREY );
              Serial.println("Realesed button ok");
            } 
        }
        else if ( PencilState )
        {
            //445, 195, 460, 180, 475, 195
            //445, 220, 460, 235, 475, 220
            if (  tftColorTable.x >= 440 && tftColorTable.x <= 470 && tftColorTable.y >= 150 && tftColorTable.y <= 190  ){
                bool down = tftColorTable.z > 200;
                if ( down ){
                    uint16_t color = tft.readPixel( tftColorTable.x , tftColorTable.y );
                    Serial.print("Color:");
                    Serial.println(color, HEX);
                    if ( color == DARKRED ){
                        tft.fillTriangle(445, 195, 460, 180, 475, 195, RED);  
                    }
                    uint8_t count = tftColorTable.PointSize;
                    bool count_One = true;
                    long tstart = millis();
                    while( down ){
                        ISPRESSED();
                        down = tftColorTable.z > 200;
                        if ( millis() - tstart > 1000 ){
                            if ( count < 4 ){
                                count++;
                                tftColorTable.PointSize = count;

                                struct textTFT t;
                                t.x = 190;
                                t.y= 295;
                                t.fcolor= BLACK;
                                t.bcolor= WHITE;
                                t.textSize = 3;
                                t.Text = "PS: " + String(tftColorTable.PointSize);
                                setUpText( t );

                                tft.fillRect(445, 200, 30, 15, WHITE);
            
                                t.x = 445;
                                t.y= 200;
                                t.fcolor= BLACK;
                                t.bcolor = WHITE ;
                                t.textSize = 2 ;
                                t.Text= String(tftColorTable.PointSize  );
                                setUpText( t );

                                
                            }
                            count_One = false;
                            tstart = millis();
                        }
                    }
                    if( count_One ){
                        if ( count < 4 ){
                            count++;
                            tftColorTable.PointSize = count;
                            struct textTFT t;
                            t.x = 190;
                            t.y= 295;
                            t.fcolor= BLACK;
                            t.bcolor= WHITE;
                            t.textSize = 3;
                            t.Text = "PS: " + String(tftColorTable.PointSize);
                            setUpText( t );

                            tft.fillRect(445, 200, 30, 15, WHITE);
            
                            t.x = 445;
                            t.y= 200;
                            t.fcolor= BLACK;
                            t.bcolor = WHITE ;
                            t.textSize = 2 ;
                            t.Text= String(tftColorTable.PointSize  );
                            setUpText( t );

                        }
                    }
                    tft.fillTriangle(445, 195, 460, 180, 475, 195, DARKRED);
                }
            }else if (  tftColorTable.x >= 440 && tftColorTable.x <= 470 && tftColorTable.y >= 210 && tftColorTable.y <= 230  ){
                bool down = tftColorTable.z > 200;
                if ( down ){
                    uint16_t color = tft.readPixel( tftColorTable.x , tftColorTable.y );
                    Serial.print("Color:");
                    Serial.println(color, HEX);
                    if ( color == DARKRED ){
                        tft.fillTriangle(445, 220, 460, 235, 475, 220, RED);  
                    }
                    uint8_t count = tftColorTable.PointSize;
                    bool count_One = true;
                    long tstart = millis();
                    while( down ){
                        ISPRESSED();
                        down = tftColorTable.z > 200;
                        if ( millis() - tstart > 1000 ){
                            if ( count > 1 ){
                                count--;
                                tftColorTable.PointSize = count;

                                struct textTFT t;
                                t.x = 190;
                                t.y= 295;
                                t.fcolor= BLACK;
                                t.bcolor= WHITE;
                                t.textSize = 3;
                                t.Text = "PS: " + String(tftColorTable.PointSize);
                                setUpText( t );

                                tft.fillRect(445, 200, 30, 15, WHITE);
            
                                t.x = 445;
                                t.y= 200;
                                t.fcolor= BLACK;
                                t.bcolor = WHITE ;
                                t.textSize = 2 ;
                                t.Text= String(tftColorTable.PointSize  );
                                setUpText( t );

                                
                            }
                            count_One = false;
                            tstart = millis();
                        }
                    }
                    if( count_One ){
                        if ( count > 1 ){
                            count--;
                            tftColorTable.PointSize = count;
                            struct textTFT t;
                            t.x = 190;
                            t.y= 295;
                            t.fcolor= BLACK;
                            t.bcolor= WHITE;
                            t.textSize = 3;
                            t.Text = "PS: " + String(tftColorTable.PointSize);
                            setUpText( t );

                            tft.fillRect(445, 200, 30, 15, WHITE);
            
                            t.x = 445;
                            t.y= 200;
                            t.fcolor= BLACK;
                            t.bcolor = WHITE ;
                            t.textSize = 2 ;
                            t.Text= String(tftColorTable.PointSize  );
                            setUpText( t );
                            
                        }
                    }
                    tft.fillTriangle(445, 220, 460, 235, 475, 220, DARKRED);
                }
            }
            bool down = tftColorTable.z > 200;
            button_ok.press(down && button_ok.contains(tftColorTable.x, tftColorTable.y));
            if (button_ok.justPressed()) {
              button_ok.drawButton(true);
              Serial.println("Pressed button ok");
              while( !button_ok.justReleased()){
                ISPRESSED();
                down = tftColorTable.z > 200;
                button_ok.press(down && button_ok.contains(tftColorTable.x, tftColorTable.y));
              }
              button_ok.drawButton();
                RedState = false;
                GreenState = false;
                BlueState = false;
                PencilState = false;
              tft.fillRect( 440 , 0 , 40, tft.height(), GREY );
              Serial.println("Realesed button ok");
            }
        }
    }
    else if ( tftColorTable.y >= 280 && SCREEN == 2  )
    {
        if ( verifyRedColor() && !RedState ){
            tft.fillRect( 440 , 0 , 40, tft.height(), GREY );
            Serial.println( "Red square" );
            RedState = true;
            GreenState = false;
            BlueState = false;
            PencilState = false;
            //Primer rectangulo
            int firstRectangleYpos = 240;
            int firstRectangleHeigth = map( tftColorTable.red, 0, 255 , 0, 230 );

            int SecondRectangleYpos = map( tftColorTable.red, 0, 255 , firstRectangleYpos, 10 );
            int SecondRectangleHeigth = 230 - firstRectangleHeigth;
            
            tft.fillRect( 450, firstRectangleYpos - firstRectangleHeigth, 20, firstRectangleHeigth, RED );
            tft.fillRect( 450, 10, 20, SecondRectangleHeigth, BLACK );

            button_ok.initButton( &tft, 460, 260 , 30, 30, BLACK, BLUE, WHITE, "OK", 1);
            button_ok.drawButton(false);

        }
        else if ( verifyGreenColor() && !GreenState ){
            tft.fillRect( 440 , 0 , 40, tft.height(), GREY );
            Serial.println( "Green square" );
            RedState = false;
            GreenState = true;
            BlueState = false;
            PencilState = false;
            //Primer rectangulo
            int firstRectangleYpos = 240;
            int firstRectangleHeigth = map( tftColorTable.green, 0, 255 , 0, 230 );

            int SecondRectangleYpos = map( tftColorTable.green, 0, 255 , firstRectangleYpos, 10 );
            int SecondRectangleHeigth = 230 - firstRectangleHeigth;
            
            tft.fillRect( 450, firstRectangleYpos - firstRectangleHeigth, 20, firstRectangleHeigth, GREEN );
            tft.fillRect( 450, 10, 20, SecondRectangleHeigth, BLACK );

            button_ok.initButton( &tft, 460, 260 , 30, 30, BLACK, BLUE, WHITE, "OK", 1);
            button_ok.drawButton(false);
        }
        else if ( verifyBlueColor() && !BlueState){
            tft.fillRect( 440 , 0 , 40, tft.height(), GREY );
            Serial.println( "Blue square" );
            RedState = false;
            GreenState = false;
            BlueState = true;
            PencilState = false;

            //Primer rectangulo
            int firstRectangleYpos = 240;
            int firstRectangleHeigth = map( tftColorTable.blue, 0, 255 , 0, 230 );

            int SecondRectangleYpos = map( tftColorTable.blue, 0, 255 , firstRectangleYpos, 10 );
            int SecondRectangleHeigth = 230 - firstRectangleHeigth;
            
            tft.fillRect( 450, firstRectangleYpos - firstRectangleHeigth, 20, firstRectangleHeigth, BLUE );
            tft.fillRect( 450, 10, 20, SecondRectangleHeigth, BLACK );

            button_ok.initButton( &tft, 460, 260 , 30, 30, BLACK, BLUE, WHITE, "OK", 1);
            button_ok.drawButton(false);
        }
        else if( verifyPecilSize() && !PencilState )
        {
            tft.fillRect( 440 , 0 , 40, tft.height(), GREY );
            Serial.println( "Pencil square" );
            RedState = false;
            GreenState = false;
            BlueState = false;
            PencilState = true;
            
            tft.fillTriangle(445, 195, 460, 180, 475, 195, DARKRED);
            tft.fillRect(445, 200, 30, 15, WHITE);
            struct textTFT t;
            t.x = 445;
            t.y= 200;
            t.fcolor= BLACK;
            t.bcolor = WHITE ;
            t.textSize = 2 ;
            t.Text= String(tftColorTable.PointSize  );
            setUpText( t );

            tft.fillTriangle(445, 220, 460, 235, 475, 220, DARKRED);

            button_ok.initButton( &tft, 460, 260 , 30, 30, BLACK, BLUE, WHITE, "OK", 1);
            button_ok.drawButton(false);

        }
        else{
            //Serial.println("X:" + String(tftColorTable.x) + " Y:" + String(tftColorTable.y) );
            bool down = tftColorTable.z > 200;
            button_refresh.press(down && button_refresh.contains(tftColorTable.x, tftColorTable.y));
            if (button_refresh.justPressed() && !button_refresh_pressed) {
              button_refresh.drawButton(true);
              button_refresh_pressed = true;
              Serial.println("Pressed button");
              while( button_refresh_pressed){
                ISPRESSED();
                down = tftColorTable.z > 200;
                button_refresh.press(down && button_refresh.contains(tftColorTable.x, tftColorTable.y));
                if (button_refresh.justReleased() ) button_refresh_pressed = false;
              }
              button_refresh.drawButton();
              tft.fillRect(0, 0, tft.width() - 40, 280, WHITE);
              button_refresh_pressed = false;
              Serial.println("Realesed button");
            } 
        }

      }    
    }
  while (ISPRESSED() == false) {
  }

}

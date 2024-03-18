#include <Arduino.h>
#include <MCUFRIEND_kbv.h>

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

#define LOGO_POS_X 45
#define LOGO_POS_Y 165

#define LOGOTEXT_POS_X 85
#define LOGOTEXT_POS_Y 285

#define LOGO_SIZE_TEXT 3

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

void logoScreen()
{
    struct textTFT t;
    t.x = LOGO_POS_X;
    t.y = LOGO_POS_Y;
    t.fcolor = ARDUINO_TURQUOISE;
    t.bcolor = WHITE;
    t.textSize = LOGO_SIZE_TEXT;
    t.Text = "Here's the Arduino Logo";
    setUpText( t );

    // Set the Logo
    t.x = LOGOTEXT_POS_X;
    t.y= LOGOTEXT_POS_Y;
    t.Text = "TFT Graphic Table";
    setUpText( t );
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
    // Set Up Logo Animation
    logoScreen();
    delay(3000);
    // White Screen
    tft.fillScreen(WHITE);
    delay(500);
}

void loop()
{
    
}

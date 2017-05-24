#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_TFTLCD.h> // Hardware-specific library
#include <TouchScreen.h>

#if defined(__SAM3X8E__)
    #undef __FlashStringHelper::F(string_literal)
    #define F(string_literal) string_literal
#endif

#define YP A3
#define XM A2
#define YM 9  
#define XP 8 

#define TS_MINX 150
#define TS_MINY 120
#define TS_MAXX 920
#define TS_MAXY 940

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
#define LCD_RESET A6

#define  BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF


Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

#define BOXSIZE 40
#define PENRADIUS 3

#define tempPin  A5
#define pressPin A4

int dispenseAmount = 16;
int buttonPress = 0;
float temp = 70.0;
float pres = 0.0;

void setup(void) {
  Serial.begin(9600);
  tft.reset();
  uint16_t identifier = tft.readID();
  tft.begin(identifier);
    tft.setRotation(3);

  //Color in buttons for UI
  tft.fillScreen(BLACK);
  pinMode(13, OUTPUT);
  tft.fillRect(tft.width()-50, 0, 50, tft.height()/2, RED);
  tft.fillRect(tft.width()-50, tft.height()/2, 50, tft.height()/2, BLUE);
  tft.fillRect(0, tft.height()-50, tft.width()-50, 50, GREEN);

  //Puts text for buttons
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.setTextWrap(false);
  tft.setCursor(tft.width()-35, 70);
  tft.print("up");
  tft.setCursor(tft.width()-40, 70+tft.height()/2);
  tft.print("dwn");
  tft.setCursor(tft.width()/2-140, tft.height()-30);
  tft.print("Dispense amount:");
  tft.print(dispenseAmount);
  tft.print("oz");
  tft.setCursor(0, 0);
  tft.setTextSize(6);
  tft.setTextColor(YELLOW);
  tft.print("Root Beer\nIs Good\nFor You!");
  tft.setCursor(0, tft.height()-70);
  tft.fillRect(0, tft.height()-70, tft.width()-50, 20, MAGENTA);
  tft.setTextSize(2);
  tft.setTextColor(RED);
  tft.print("Temperature:");
  tft.print(temp);
  tft.print("F  Pressure:");
  tft.print(pres);
  tft.print("psi");
  
}

#define MINPRESSURE 40
#define MAXPRESSURE 1000

int tempOut = 0;

void loop()
{
  digitalWrite(13, HIGH);
  TSPoint p = ts.getPoint();
  digitalWrite(13, LOW);

  

  
  // if sharing pins, you'll need to fix the directions of the touchscreen pins
  //pinMode(XP, OUTPUT);
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  //pinMode(YM, OUTPUT);

  if(tempOut >= 1000)
  {
    float presV = analogRead(pressPin);
    presV *= 5.0;
    presV /= 1024.0;
    pres = (presV - .5)*75.0/300.0;
    float tempRead = analogRead(tempPin);
    float tempV = tempRead * 5.0;
    tempV /= 1024.0;
    temp = ((tempV-0.5) * 100) * (9.0/5.0) + 32.0;
    tft.fillRect(0, tft.height()-70, tft.width()-50, 20, MAGENTA);
    tft.setCursor(0, tft.height()-70);
    tft.setTextSize(2);
    tft.setTextColor(RED);
    tft.print("Temperature:");
    tft.print(temp);
    tft.print("F  Pressure:");
    tft.print(pres);
    tft.print("psi");
    tempOut = 0;
  }
  else
    tempOut++;
  

  if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
    p.x = map(p.x, TS_MAXX, TS_MINX, tft.height(), 0);
    p.y = map(p.y, TS_MINY, TS_MAXY, tft.width(), 0);
    if (p.y > tft.width()-50  && p.x < tft.height()/2) 
    {
      Serial.println("Dispense More!");
      if(dispenseAmount < 32)
      {
        tft.fillRect(0, tft.height()-50, tft.width()-50, 50, GREEN);
        dispenseAmount++;
        tft.setTextColor(WHITE);
        tft.setTextSize(2);
        tft.setCursor(tft.width()/2-140, tft.height()-30);
        tft.print("Dispense amount:");
        tft.print(dispenseAmount);
        tft.print("oz");
      }
      delay(100);
    }
    if (p.y > tft.width()-50 && p.x > tft.height()/2) 
    {
      Serial.println("Dispense Less!");
      if(dispenseAmount > 1)
      {
        tft.fillRect(0, tft.height()-50, tft.width()-50, 50, GREEN);
        dispenseAmount--;
        tft.setTextColor(WHITE);
        tft.setTextSize(2);
        tft.setCursor(tft.width()/2-140, tft.height()-30);
        tft.print("Dispense amount:");
        tft.print(dispenseAmount);
        tft.print("oz");
      }
      delay(100);
    }
    if(p.y < tft.width()-50 && p.x > tft.height()-50 && buttonPress == 0)
    {
      Serial.println("Dispense!!!");
      tft.fillRect(0, tft.height()-50, tft.width()-50, 50, GREEN);
      dispenseAmount--;
      tft.setTextColor(WHITE);
      tft.setTextSize(2);
      tft.setCursor(tft.width()/2-140, tft.height()-30);
      tft.print("Dispensing beverage!");
      delay(1000);
       tft.fillRect(0, tft.height()-50, tft.width()-50, 50, GREEN);
        dispenseAmount--;
        tft.setTextColor(WHITE);
        tft.setTextSize(2);
        tft.setCursor(tft.width()/2-140, tft.height()-30);
        tft.print("Dispense amount:");
        tft.print(dispenseAmount);
        tft.print("oz");
      
      //add check to see if keg is ready to dispense again
      //have delay here just for now
    }
 
    /*
    Serial.print("("); Serial.print(p.x);
    Serial.print(", "); Serial.print(p.y);
    Serial.println(")");
    */
    delay(100);
  }
}

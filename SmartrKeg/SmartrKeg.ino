
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
#define sol 10

#define TS_MINX 150
#define TS_MINY 120
#define TS_MAXX 920
#define TS_MAXY 940

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
#define LCD_RESET A4

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


#define FLOWSENSORPIN 11

// count how many pulses!
volatile uint16_t pulses = 0;
// track the state of the pulse pin
volatile uint8_t lastflowpinstate;
// you can try to keep time of how long it is between pulses
volatile uint32_t lastflowratetimer = 0;
// and use that to calculate a flow rate
volatile float flowrate;
// Interrupt is called once a millisecond, looks for any pulses from the sensor!
SIGNAL(TIMER0_COMPA_vect) {
  uint8_t x = digitalRead(FLOWSENSORPIN);

  if (x == lastflowpinstate) {
    lastflowratetimer++;
    return; // nothing changed!
  }

  if (x == HIGH) {
    //low to high transition!
    pulses++;
  }
  lastflowpinstate = x;
  flowrate = 1000.0;
  flowrate /= lastflowratetimer;  // in hertz
  lastflowratetimer = 0;
}

void useInterrupt(boolean v) {
  if (v) {
    // Timer0 is already used for millis() - we'll just interrupt somewhere
    // in the middle and call the "Compare A" function above
    OCR0A = 0xAF;
    TIMSK0 |= _BV(OCIE0A);
  } else {
    // do not call the interrupt function COMPA anymore
    TIMSK0 &= ~_BV(OCIE0A);
  }
}



int dispenseAmount = 16;
int buttonPress = 0;
int temp = 70;
int pres = 75;

void setup(void) {
  Serial.begin(9600);
  tft.reset();
  uint16_t identifier = tft.readID();
  tft.begin(identifier);
  tft.setRotation(3);

  tft.fillScreen(BLACK);
  pinMode(13, OUTPUT);
  tft.fillRect(tft.width() - 50, 0, 50, tft.height() / 2, RED);
  tft.fillRect(tft.width() - 50, tft.height() / 2, 50, tft.height() / 2, BLUE);
  tft.fillRect(0, tft.height() - 50, tft.width() - 50, 50, GREEN);

  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.setTextWrap(false);
  tft.setCursor(tft.width() - 35, 70);
  tft.print("up");
  tft.setCursor(tft.width() - 40, 70 + tft.height() / 2);
  tft.print("dwn");
  tft.setCursor(tft.width() / 2 - 140, tft.height() - 30);
  tft.print("Dispense amount:");
  tft.print(dispenseAmount);
  tft.print("oz");
  tft.setCursor(0, 0);
  tft.setTextSize(6);
  tft.setTextColor(YELLOW);
  tft.print("Root Beer\nIs Good\nFor You!");
  tft.setCursor(0, tft.height() - 70);
  tft.setTextSize(2);
  tft.setTextColor(RED);
  tft.print("Temperature:");
  tft.print(temp);
  tft.print("F  Pressure:");
  tft.print(pres);
  tft.print("psi");


  pinMode(FLOWSENSORPIN, INPUT);
  digitalWrite(FLOWSENSORPIN, HIGH);
  lastflowpinstate = digitalRead(FLOWSENSORPIN);
  useInterrupt(true);

}

#define MINPRESSURE 40
#define MAXPRESSURE 1000

void loop()
{
  Serial.print("Freq: "); Serial.println(flowrate);
  Serial.print("Pulses: "); Serial.println(pulses, DEC);
  float ounces = pulses;
  ounces /= 7.5;
  ounces /= 60.0;
  ounces *= 33.814;
  Serial.print(ounces); Serial.println(" ounces");



  digitalWrite(13, HIGH);
  TSPoint p = ts.getPoint();
  digitalWrite(13, LOW);

  pinMode(sol, OUTPUT);
  // if sharing pins, you'll need to fix the directions of the touchscreen pins
  //pinMode(XP, OUTPUT);
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  //pinMode(YM, OUTPUT);

  if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
    p.x = map(p.x, TS_MAXX, TS_MINX, tft.height(), 0);
    p.y = map(p.y, TS_MINY, TS_MAXY, tft.width(), 0);
    if (p.y > tft.width() - 50  && p.x < tft.height() / 2)
    {
      Serial.println("Dispense More!");
      if (dispenseAmount < 32)
      {
        tft.fillRect(0, tft.height() - 50, tft.width() - 50, 50, GREEN);
        dispenseAmount++;
        tft.setTextColor(WHITE);
        tft.setTextSize(2);
        tft.setCursor(tft.width() / 2 - 140, tft.height() - 30);
        tft.print("Dispense amount:");
        tft.print(dispenseAmount);
        tft.print("oz");
      }
      delay(100);
    }
    if (p.y > tft.width() - 50 && p.x > tft.height() / 2)
    {
      Serial.println("Dispense Less!");
      if (dispenseAmount > 1)
      {
        tft.fillRect(0, tft.height() - 50, tft.width() - 50, 50, GREEN);
        dispenseAmount--;
        tft.setTextColor(WHITE);
        tft.setTextSize(2);
        tft.setCursor(tft.width() / 2 - 140, tft.height() - 30);
        tft.print("Dispense amount:");
        tft.print(dispenseAmount);
        tft.print("oz");
      }
      delay(100);
    }
    if (p.y < tft.width() - 50 && p.x > tft.height() - 50 && buttonPress == 0)
    {
      Serial.println("Dispense!!!");
      digitalWrite(sol, HIGH);
      tft.fillRect(0, tft.height() - 50, tft.width() - 50, 50, GREEN);
      tft.setTextColor(WHITE);
      tft.setTextSize(2);
      tft.setCursor(tft.width() / 2 - 140, tft.height() - 30);
      tft.print("Dispensing beverage!");
      while(ounces <= dispenseAmount)
      {
        Serial.print("Freq: "); Serial.println(flowrate);
        Serial.print("Pulses: "); Serial.println(pulses, DEC);
        float ounces = pulses;
        ounces /= 7.5;
        ounces /= 60.0;
        ounces *= 33.814;
        Serial.print(ounces); Serial.print(" ounces"); Serial.print(dispenseAmount); Serial.println(" should be this");
        if(ounces > dispenseAmount)
        {
          pulses = 0;
          ounces = 0;
          break;
        }
        delay(100);
      }
      tft.fillRect(0, tft.height() - 50, tft.width() - 50, 50, GREEN);
      tft.setTextColor(WHITE);
      tft.setTextSize(2);
      tft.setCursor(tft.width() / 2 - 140, tft.height() - 30);
      tft.print("Dispense amount:");
      tft.print(dispenseAmount);
      tft.print("oz");
      digitalWrite(sol, LOW);

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

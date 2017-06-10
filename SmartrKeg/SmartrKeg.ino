//Name: David Zhu
//SID: 861236820
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_TFTLCD.h> // Hardware-specific library
#include <TouchScreen.h>

#if defined(__SAM3X8E__)
    #undef __FlashStringHelper::F(string_literal)
    #define F(string_literal) string_literal
#endif

//pins used for touchscreen
#define YP A3
#define XM A2
#define YM 9  
#define XP 8 

#define TS_MINX 150
#define TS_MINY 120
#define TS_MAXX 920
#define TS_MAXY 940

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

//pins used for display
#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
#define LCD_RESET A6

//Commonly used colors
#define BLACK   0x0000
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
#define solPin   10
#define flowPin  11

int dispenseAmount = 16; //amount to dispense
int buttonPress = 0; //detects if "dispense" button has been pressed
float temp = 70.0; //temperature within the keg
float pres = 0.0; //pressure within the keg
char description[100] = "Root Beer IsGood For    You!";

//Library code for the flow meter
volatile uint16_t pulses = 0;
 // track the state of the pulse pin
volatile uint8_t lastflowpinstate;
// you can try to keep time of how long it is between pulses
volatile uint32_t lastflowratetimer = 0;
// and use that to calculate a flow rate
volatile float flowrate;
// Interrupt is called once a millisecond, looks for any pulses from the sensor!

SIGNAL(TIMER0_COMPA_vect) {
   uint8_t x = digitalRead(flowPin);
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

//Refreshes the "dispense" button area
void refreshDispense() 
{
    tft.fillRect(0, tft.height()-50, tft.width()-50, 50, GREEN);
    tft.setTextColor(WHITE);
    tft.setTextSize(2);
    tft.setCursor(tft.width()/2-140, tft.height()-30);
    tft.print("Dispense amount:");
    tft.print(dispenseAmount);
    tft.print("oz"); 
}

//Overwrites the description area with a low pressure error
void refreshPressErr()
{
    tft.fillRect(0, 0, tft.width()-50, tft.height()-70, BLACK);
    tft.setCursor(0, 0);
    tft.setTextSize(6);
    tft.setTextColor(RED);
    tft.print("PRESSURE IS \nLOW    ");
}

//Overwrites the description area with a high temperature error
void refreshTempErr()
{
    tft.fillRect(0, 0, tft.width()-50, tft.height()-70, BLACK);
    tft.setCursor(0, 0);
    tft.setTextSize(6);
    tft.setTextColor(RED);
    tft.print("TEMP IS \nHIGH    ");
}

//Overwrites the description area with a high temperature and low pressure error
void refreshAllErr()
{
    tft.fillRect(0, 0, tft.width()-50, tft.height()-70, BLACK);
    tft.setCursor(0, 0);
    tft.setTextSize(6);
    tft.setTextColor(RED);
    tft.print(" EVERYTHING\nIS\nWRONG    ");
}

//Refreshes the area that displays the current temperature and pressure
void refreshPress()
{
    tft.fillRect(0, tft.height()-70, tft.width()-50, 20, BLACK);
    tft.setCursor(0, tft.height()-70);
    tft.setTextSize(2);
    tft.setTextColor(WHITE);
    tft.print("Temp:");
    tft.print(temp);
    tft.print("F Pressure:");
    tft.print(pres);
    tft.print("psi");
}

char line1 = 0;
char line2 = 0;
char line3 = 0;
char line4 = 0;
char line5 = 0;
//Refreshes the description area
void refreshDescr()
{
    tft.fillRect(0, 0, tft.width()-50, tft.height()-70, BLACK);
    tft.setCursor(0, 0);
    tft.setTextSize(6);
    tft.setTextColor(YELLOW);
    for(line1 = 0; line1 < 12; line1++)
      tft.print(description[line1]);
    tft.print("\n");
    for(line2 = line1; line2 < 24; line2++)
      tft.print(description[line2]);
    tft.print("\n");
    for(line3 = line2; line3 < 36; line3++)
      tft.print(description[line3]);
    tft.print("\n");
    for(line4 = line3; line4 < 48; line4++)
      tft.print(description[line4]);
    tft.print("\n");
    for(line5 = line4; line5 < 60; line5++)
      tft.print(description[line5]); 
}

void refreshDescrN()
{
    tft.fillRect(0, 0, tft.width()-50, tft.height()-70, BLACK);
    tft.setTextSize(6);
    tft.setTextColor(YELLOW);
    tft.print(description);
}

//Places buttons on the screen for increasing/decreasing output volume
void refreshInitial()
{
    //Puts colors for buttons
    tft.fillRect(tft.width()-50, 0, 50, tft.height()/2, RED);
    tft.fillRect(tft.width()-50, tft.height()/2, 50, tft.height()/2, BLUE);

    //Puts text for buttons
    tft.setTextColor(WHITE);
    tft.setTextSize(2);
    tft.setTextWrap(false);
    tft.setCursor(tft.width()-35, 70);
    tft.print("up");
    tft.setCursor(tft.width()-40, 70+tft.height()/2);
    tft.print("dwn");
    refreshDispense();
    refreshDescr();
    refreshPress();
}

void setup(void) {
  
  Serial.begin(9600);
  pinMode(solPin, OUTPUT);
  pinMode(flowPin, INPUT);
  digitalWrite(flowPin, HIGH);
  lastflowpinstate = digitalRead(flowPin);
  useInterrupt(true);
  
  tft.reset();
  uint16_t identifier = tft.readID();
  tft.begin(identifier);
  tft.setRotation(3);

  //Color in buttons for UI
  tft.fillScreen(BLACK);
  tft.fillRect(tft.width()-50, 0, 50, tft.height()/2, RED);
  tft.fillRect(tft.width()-50, tft.height()/2, 50, tft.height()/2, BLUE);

  //Puts text for buttons
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.setTextWrap(false);
  tft.setCursor(tft.width()-35, 70);
  tft.print("up");
  tft.setCursor(tft.width()-40, 70+tft.height()/2);
  tft.print("dwn");
  refreshDispense();
  tft.setCursor(0,0);
  refreshDescr();
  refreshPress();


  delay(100);
  float presV = analogRead(pressPin);
  presV *= 5.0;
  presV /= 1024.0;
  pres = (presV-.24)*75.0;
  float tempRead = analogRead(tempPin);
  float tempV = tempRead * 5.0;
  tempV /= 1024.0;
  temp = ((tempV-0.5) * 100) * (9.0/5.0) + 32.0;
  refreshPress();
}

#define MINPRESSURE 40
#define MAXPRESSURE 1000

int tempCount = 0;
char input;
bool descrOn = true;
void loop()
{
  while(Serial.available()>0)
  {
    delay(100);
    memset(description, 0, sizeof(description));  
    input = char(Serial.read());
   // while(input != '!')
    for(int counterIn = 0; counterIn < 60; counterIn++)
    {
      description[counterIn] = input;
      input = char(Serial.read());
    }
    Serial.print(description);
      //tft.print(description[i]);
    refreshDescr();
    
    if(Serial.available() <= 0)
    {
       delay(300);
      // description = "I CLOSED";
      // refreshDescr();
    }
  }
  
//  Serial.print("Freq: "); Serial.println(flowrate);
 // Serial.print("Pulses: "); Serial.println(pulses, DEC);
  float ounces = pulses;
  ounces /= 7.5;
  ounces /= 60.0;
  ounces *= 33.814;
 // Serial.print(ounces); Serial.println(" ounces");
  
  digitalWrite(13, HIGH);
  TSPoint p = ts.getPoint();
  digitalWrite(13, LOW);

  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);

  if(tempCount >= 3000)
  {
    float presV = analogRead(pressPin);
    presV *= 5.0;
    presV /= 1024.0;
    pres = (presV-.24)*75.0;
    float tempRead = analogRead(tempPin);
    float tempV = tempRead * 5.0;
    tempV /= 1024.0;
    temp = ((tempV-0.5) * 100) * (9.0/5.0) + 32.0;
    //If statements to alert user of temperature or pressure errors
    if(temp > 76 && pres < 0)
    {
        refreshAllErr();
        descrOn = false;
    }
    else if(temp > 76)
    {
        refreshTempErr();
        descrOn = false;
    }
    else if(pres < 0)
    {
        refreshPressErr();
        descrOn = false;
    }
    else if(!descrOn)
    {
        refreshDescr();
        descrOn = true;
    }
    refreshPress();
    tempCount = 0;
  }
  else
    tempCount++;
  //If the touchscreen is touched with enough pressure
  if (p.z > MINPRESSURE && p.z < MAXPRESSURE) 
  {
    p.x = map(p.x, TS_MAXX, TS_MINX, tft.height(), 0);
    p.y = map(p.y, TS_MINY, TS_MAXY, tft.width(), 0);
    //Dispense more button is pushed
    if (p.y > tft.width()-50  && p.x < tft.height()/2) 
    {
    //  Serial.println("Dispense More!");
      if(dispenseAmount < 32)
      { 
        dispenseAmount++;
        refreshDispense();
      }
      delay(100);
    }
    //Dispense less button is pushed
    if (p.y > tft.width()-50 && p.x > tft.height()/2) 
    {
     // Serial.println("Dispense Less!");
      if(dispenseAmount > 1)
      {
        dispenseAmount--;
        refreshDispense();
      }
      delay(100);
    }
    //Dispense button is pushed
    if(p.y < tft.width()-50 && p.x > tft.height()-50 && buttonPress == 0)
    {
     // Serial.println("Dispense!!!");
      tft.fillRect(0, tft.height()-50, tft.width()-50, 50, GREEN);
      tft.setTextColor(WHITE);
      tft.setTextSize(2);
      tft.setCursor(tft.width()/2-140, tft.height()-30);
      tft.print("Dispensing beverage!");
      digitalWrite(solPin, HIGH); //Turns solenoid ON
      while(ounces <= dispenseAmount)
      {
      //  Serial.print("Freq: "); Serial.println(flowrate);
       // Serial.print("Pulses: "); Serial.println(pulses, DEC);
        float ounces = pulses;
        ounces /= 7.5;
        ounces /= 60.0;
        ounces *= 33.814;
      //  Serial.print(ounces); Serial.print(" ounces"); Serial.print(dispenseAmount); Serial.println(" should be this");
        if(ounces > dispenseAmount)
        {
          digitalWrite(solPin, LOW);//Turns solenoid OFF
          break;
        }
      }
      delay(3000); //3 second delay after turning solenoid off before resetting values in case of drippage
      dispenseAmount = 16;
      pulses = 0;
      ounces = 0;
      refreshDispense();
    }
 
    /*
    Serial.print("("); Serial.print(p.x);
    Serial.print(", "); Serial.print(p.y);
    Serial.println(")");
    */
    delay(100);
  }
}

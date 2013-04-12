#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "RTClib.h"

#define OLED_DC 11
#define OLED_CS 12
#define OLED_CLK 10
#define OLED_MOSI 9
#define OLED_RESET 13

RTC_DS1307 RTC;
Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

#define LOGO16_GLCD_HEIGHT 16 
#define LOGO16_GLCD_WIDTH  16 

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

// THIS IS A SKETCH FOR A WEARABLE BLUETOOTH WATCH / CALLER ID
// Hardware used:
//   Arduino Fio: I used the OSEPP: http://osepp.com/products/arduino-compatible-boards/osepp-fio-arduino-compatible/
//   Real-Time Clock: https://www.sparkfun.com/products/99
//   EWS Bluegiga WT32 breakout board: http://embeddedwirelesssolutions.com/bluegiga_wt32_breakout_board
//   Adafruit 1.3" OLED graphic display: http://www.adafruit.com/products/938#Learn
//   RadioShack 3VDC vibration motor: http://www.radioshack.com/product/index.jsp?productId=2914700
//   1 AA battery
//   iPod Battery

int resetpin = 6;
int vibropin = 4;

boolean showClock = true;

char wt32Input[100] = {0};
char wt32InputReceived[100] = {0};
char lastLog[100] = {0};
int wt32InputIndex = 0;
int phoneNumberLen = 0;

boolean ringing = false;
boolean ringSent = false;

long startRingTime;
int loopCount = 0;

void setup()   {                
  Serial.begin(9600);
  Wire.begin();
  RTC.begin();
//  if (! RTC.isrunning()) {
//    Serial.println("RTC is NOT running!");
//----following line sets the RTC to the date & time this sketch was compiled
//----this is how you set the time on the RTC
//    RTC.adjust(DateTime(__DATE__, __TIME__));
//  }
  
  display.begin(SSD1306_SWITCHCAPVCC);
  
  display.display(); // show splashscreen
  delay(2000);
  display.clearDisplay();   // clears the screen and buffer
  
  pinMode(vibropin, OUTPUT);
  pinMode(resetpin, OUTPUT);
  digitalWrite(resetpin, LOW);
  delay(200);
  digitalWrite(resetpin, HIGH);
  delay(200);
  digitalWrite(resetpin, LOW);
  delay(200);
//  for configuring BT (should only have to do this once):
//  Serial.println("SET BT CLASS 200408");
//  delay(200);
//  Serial.println("SET PROFILE HFP ON");
//  delay(200);
//  Serial.println("SET PROFILE SPP OFF");
//  delay(200);
//  Serial.println("SET PROFILE PBAP ON");
//  delay(200);
//  Serial.println("SET CONTROL AUTOCALL 111E 5000 HFP");
//  delay(200);
}

void loop() {
  
  if (ringing) {
    if (!ringSent) {
      digitalWrite(vibropin, HIGH);
      ringSent = true;
      startRingTime = millis();
    } else {
      if (millis() - startRingTime > 1000) {
        digitalWrite(vibropin, LOW);
      }
      if (millis() - startRingTime > 2000) {
        ringSent = false;
      }
    }
  } else {
    digitalWrite(vibropin, LOW);
  }
//LISTEN TO WT32
  if (Serial.available()) {
    wt32Input[wt32InputIndex] = Serial.read(); 
    if (wt32Input[wt32InputIndex] == '\n') {
      strcpy(wt32InputReceived, wt32Input); 
      wt32InputIndex = 0; 
    } else { 
      wt32InputIndex++; 
    } 
  }
  if (stringCompare(wt32InputReceived, "HFP 0 RING")) {
    if (!ringing) {
      ringing = true;
    }
    showClock = false;
  }
  if (stringCompare(wt32InputReceived, "HFP 0 STATUS \"callsetup\" 0")) {
    ringing = false;
    ringSent = false;
    digitalWrite(vibropin, LOW);
    showClock = true;
  }
  if (stringCompare(wt32InputReceived, "NO CARRIER 0")) {
    display.setTextSize(.5);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.clearDisplay();
    display.println("NO CARRIER");
    display.display();
    showClock = true;
  }
  if (stringCompare(wt32InputReceived, "HFP 0 CALLERID")) {
    // String should look like:
    //HFP 0 CALLERID "5125774186" "IfThisThenThat"
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0,32);
    display.clearDisplay();
    showClock = false;
    
    int startIndex = 15;
    char stopCharacter = ' ';
    if (wt32InputReceived[startIndex] == '"') {
      stopCharacter = '"';
    }
    startIndex++;
    for (int i = startIndex;i<100;i++) {
      if (wt32InputReceived[i] != stopCharacter) {
        display.print(wt32InputReceived[i]);
      } else {
        display.println("");
        startIndex = i + 2;
        i = 101;
      }
    }
    display.setCursor(0,0);
    if (wt32InputReceived[startIndex] == '"') {
      stopCharacter = '"';
    } else {
      stopCharacter = ' ';
    }
    startIndex++;
    for (int i = startIndex;i<100;i++) {
      if (wt32InputReceived[i] != stopCharacter) {
        display.print(wt32InputReceived[i]);
      } else {
        display.println("");
        i = 101;
      }
    }
    display.display();
  }
  strcpy(wt32InputReceived,'\0');
  
  loopCount++;
  if (loopCount == 0 || (loopCount > 5000 && showClock)) {
    doDrawClock();
    loopCount = 1;
  }
}

void doDrawClock() {
  DateTime now = RTC.now();
  int minutes = now.minute();
  int hours = now.hour();
  display.clearDisplay();
  display.drawLine(0, 16, 6, 18, WHITE);
  display.drawLine(33, 0, 37, 4, WHITE);
  display.drawLine(64, 0, 64, 4, WHITE);
  display.drawLine(97, 0, 93, 4, WHITE);
  display.drawLine(118, 18, 128, 16, WHITE);
  display.drawLine(0, 31, 6, 31, WHITE);
  display.drawLine(118, 31, 128, 31, WHITE);
  display.drawLine(118, 47, 128, 49, WHITE);
  display.drawLine(93, 59, 97, 64, WHITE);
  display.drawLine(64, 59, 64, 64, WHITE);
  display.drawLine(37, 59, 33, 64, WHITE);
  display.drawLine(0, 49, 6, 47, WHITE);
  display.setCursor(60,28);
  
  int mins = minutes - minutes%5;
  switch (mins) {
    case 0 :
      display.drawLine(64, 32, 64, 2, WHITE);
      break;
    case 5 :
      display.drawLine(64, 32, 85, 11, WHITE);
      break;
    case 10 :
      display.drawLine(64, 32, 92, 25, WHITE);
      break;
    case 15 :
      display.drawLine(64, 32, 93, 32, WHITE);
      break;
    case 20 :
      display.drawLine(64, 32, 92, 39, WHITE);
      break;
    case 25 :
      display.drawLine(64, 32, 85, 53, WHITE);
      break;
    case 30 :
      display.drawLine(64, 32, 64, 62, WHITE);
      break;
    case 35 :
      display.drawLine(64, 32, 43, 53, WHITE);
      break;
    case 40 :
      display.drawLine(64, 32, 36, 39, WHITE);
      break;
    case 45 :
      display.drawLine(64, 32, 2, 32, WHITE);
      break;
    case 50 :
      display.drawLine(64, 32, 36, 25, WHITE);
      break;
    case 55 :
      display.drawLine(64, 32, 43, 11, WHITE);
      break;
  }
  
  switch(hours%12) {
    case 0 :
      display.drawLine(64, 32, 64, 13, WHITE);
      break;
    case 1 :
      display.drawLine(64, 32, 77, 19, WHITE);
      break;
    case 2 :
      display.drawLine(64, 32, 82, 25, WHITE);
      break;
    case 3 :
      display.drawLine(64, 32, 83, 32, WHITE);
      break;
    case 4 :
      display.drawLine(64, 32, 82, 39, WHITE);
      break;
    case 5 :
      display.drawLine(64, 32, 77, 44, WHITE);
      break;
    case 6 :
      display.drawLine(64, 32, 64, 50, WHITE);
      break;
    case 7 :
      display.drawLine(64, 32, 51, 44, WHITE);
      break;
    case 8 :
      display.drawLine(64, 32, 46, 39, WHITE);
      break;
    case 9 :
      display.drawLine(64, 32, 47, 32, WHITE);
      break;
    case 10 :
      display.drawLine(64, 32, 46, 25, WHITE);
      break;
    case 11 :
      display.drawLine(64, 32, 51, 19, WHITE);
      break;
  }
// digital clock face:
//  display.setTextSize(1);
//  display.setTextColor(WHITE);
//  display.setCursor(54,48);
//  display.print(now.hour());
//  display.print(":");
//  display.println(now.minute());
  display.display();   
}

boolean stringCompare(char* string1, char* string2) {
  for (int i =0; i<strlen(string2);i++) {
    if (string2[i] != '?') { // Add wildcard functionality 
      if (string1[i] != string2[i]) {
        return false; 
      }
    } else {
      return true; 
    }
  }
}

/**
 * FMiWeatherStationESP32S2.ino
 * based on examples:
 * BasicHTTPClient.ino from ESP32S2 Dev Module examples and 
 * Flash_PNG from TFT_eSPI.h examples
 * 
 *
 *  Created on: 2023-11-30
 *
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <User_Setup.h>
#include <TFT_eSPI.h>
#include <TFT_Touch.h>
#include <SPI.h>
#include <math.h>
#include <ArduinoJson.h>
#include <PNGdec.h>
#include "symbols.h" // Image is stored here in an 8 bit array
#include "SSID.h"

PNG png;
#define MAX_IMAGE_WIDTH 42 // Adjust for your images
int16_t xpos = 0; //for symbol image
int16_t ypos = 0; //for symbol image


#define USE_SERIAL Serial
#define TFT_GREY 0x5AEB
#define DARKER_GREY 0x18E3
#define SHOW_TIMER 1
#define SHOW_WEATHER 2

// http request frequency
#define REFRESH_INTERWALL 60000l
WiFiMulti wifiMulti;

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h
// These are the pins used to interface between the 2046 touch controller and Arduino Pro
#define DOUT 39  /* Data out pin (T_DO) of touch screen */
#define DIN  32  /* Data in pin (T_DIN) of touch screen */
#define DCS  33  /* Chip select pin (T_CS) of touch screen */
#define DCLK 25  /* Clock pin (T_CLK) of touch screen */

/* Create an instance of the touch screen library */
TFT_Touch touch = TFT_Touch(DCS, DCLK, DIN, DOUT);
bool swap =true;

////timer data///////
uint32_t targetTime = 0;// for next 1 second timeout
//static uint8_t conv2d(const char* p); // Forward declaration needed for IDE 1.6.x
uint8_t hh =0 /*conv2d(__TIME__)*/, mm =0 /*conv2d(__TIME__ + 3)*/, ss = 0/*conv2d(__TIME__ + 6)*/; // Get H, M, S from compile time
byte omm = 99, oss = 99;
byte xcolon = 0, xsecs = 0;
unsigned int colour = 0;
unsigned long zeroMoment = millis();
////////////////////

void setup() {
  USE_SERIAL.begin(115200);

  tft.init();
  tft.setSwapBytes(true);  
  tft.fillScreen(TFT_BLACK);
  tft.setRotation(3);
  tft.setTextSize(1);
  tft.setSwapBytes(true); 

  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  touch.setCal(526, 3443, 750, 3377, 320, 240, 1);
  int xpos = 0;
  int ypos = 85; // Top left corner ot clock text, about half way down
  int ysecs = ypos + 24;    
  for(uint8_t t = 4; t > 0; t--) {
      USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
      USE_SERIAL.flush();
      delay(1000);
  }
  wifiMulti.addAP(SECRET_SSID, SECRET_PASS);
  tft.setCursor(0, 0, 4);
  targetTime = millis() + 1000;
}


long lastRun = 0;
int whatToShow = 2;
void loop() {
    // wait for WiFi connection
    uint16_t touchX, touchY;
    bool touched = touch.Pressed();
    //touchX = touch.X();
    //touchY = touch.Y();
    if(touched){
      whatToShow= whatToShow == SHOW_TIMER?SHOW_WEATHER:SHOW_TIMER;
      swap=true;
      //USE_SERIAL.println("Touched");
    }else{
      //USE_SERIAL.println("not Touched");
    }

    switch (whatToShow)
    {
    case SHOW_TIMER:
      if(swap){
        tft.fillScreen(TFT_BLACK);
        //targetTime = millis();
        //zeroMoment = millis();
        hh=0;mm=0;ss=0;
      }
      drawTimer(swap);
      break;
    case SHOW_WEATHER:
      if(lastRun+REFRESH_INTERWALL < millis() || lastRun == 0 || swap){
        if(swap){
          tft.fillScreen(TFT_BLACK);
        }
        lastRun =millis();
        refreshForecast();
      }
    break;
    default:
      break;
    }
    if(swap){
      swap =!swap;
    }

    delay(100);

 
}


void drawTimer(bool refreshView){
  tft.setTextSize(1);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  if (targetTime < millis()) {
    // Set next update for 1 second later
    targetTime = millis() + 1000;

    // Adjust the time values by adding 1 second
    ss++;              // Advance second
    if (ss == 60) {    // Check for roll-over
      ss = 0;          // Reset seconds to zero
      omm = mm;        // Save last minute time for display update
      mm++;            // Advance minute
      if (mm > 59) {   // Check for roll-over
        mm = 0;
        hh++;          // Advance hour
        if (hh > 23) { // Check for 24hr roll-over (could roll-over on 13)
          hh = 0;      // 0 for 24 hour clock, set to 1 for 12 hour clock
        }
      }
    }


    // Update digital time
    int xpos = 0;
    int ypos = 85; // Top left corner ot clock text, about half way down
    int ysecs = ypos + 24;

    if (omm != mm || refreshView) { // Redraw hours and minutes time every minute
      omm = mm;
      // Draw hours and minutes
      if (hh < 10) xpos += tft.drawChar('0', xpos, ypos, 8); // Add hours leading zero for 24 hr clock
      xpos += tft.drawNumber(hh, xpos, ypos, 8);             // Draw hours
      xcolon = xpos; // Save colon coord for later to flash on/off later
      xpos += tft.drawChar(':', xpos, ypos - 8, 8);
      if (mm < 10) xpos += tft.drawChar('0', xpos, ypos, 8); // Add minutes leading zero
      xpos += tft.drawNumber(mm, xpos, ypos, 8);             // Draw minutes
      xsecs = xpos; // Sae seconds 'x' position for later display updates
    }
    if (oss != ss) { // Redraw seconds time every second
      oss = ss;
      xpos = xsecs;

      if (ss % 2) { // Flash the colons on/off
        tft.setTextColor(0x39C4, TFT_BLACK);        // Set colour to grey to dim colon
        tft.drawChar(':', xcolon, ypos - 8, 8);     // Hour:minute colon
        xpos += tft.drawChar(':', xsecs, ysecs, 6); // Seconds colon
        tft.setTextColor(TFT_YELLOW, TFT_BLACK);    // Set colour back to yellow
      }
      else {
        tft.drawChar(':', xcolon, ypos - 8, 8);     // Hour:minute colon
        xpos += tft.drawChar(':', xsecs, ysecs, 6); // Seconds colon
      }

      //Draw seconds
      if (ss < 10) xpos += tft.drawChar('0', xpos, ysecs, 6); // Add leading zero
      tft.drawNumber(ss, xpos, ysecs, 6);                     // Draw seconds
     
    }
  }
}

// Function to extract numbers from compile time string
static uint8_t conv2d(const char* p) {
  uint8_t v = 0;
  if ('0' <= *p && *p <= '9')
    v = *p - '0';
  return 10 * v + *++p - '0';
}

void refreshForecast(){
  if((wifiMulti.run() == WL_CONNECTED)) {
    HTTPClient http;
    http.begin("http://robo.sukelluspaikka.fi/images/weather.json"); //HTTP
    int httpCode = http.GET();
    if(httpCode > 0) {
      String payload = http.getString();
      DynamicJsonDocument doc(3072);
      DeserializationError error = deserializeJson(doc, payload);
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0, 0, 4);
      if (error) {
        tft.setTextFont(2);
        tft.print("deserializeJson() failed: ");
        tft.println(error.c_str());
        return;
      }
      int i =0;         
      for (JsonObject forecast_item : doc["forecast"].as<JsonArray>()) {
        //if(++i<10){    
        int hrs = forecast_item["hours"]; 
        if(hrs==8 ||hrs==12 ||hrs==16||hrs==20||hrs==1 || i==0){
          i=1;
          int temp = forecast_item["temp"]; 
          int wps = forecast_item["wind"]; 
          int wDir = forecast_item["windDir"];
          int symbol = forecast_item["symbol"];
          tft.setTextColor(TFT_YELLOW,TFT_BLACK);
          tft.setTextFont(6);
          tft.printf("%02d %3d ",hrs,temp );
          tft.setTextFont(4);
          tft.printf("%2d",wps);
          tft.setTextFont(2);
          tft.print(" m/s");
          tft.setTextFont(6);
          int x=tft.getCursorX();
          int y = tft.getCursorY();
          //if(swap ){
            drawWindRose(wDir,x,y);
          //}else{
            xpos =x +60;
            ypos =y +10;
            drawSymbol(symbol);
          //}                    
          tft.print('\n');
        }
      }
      swap = !swap;     
    } else {
      tft.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
    //delay(5000);
  }
}

void drawWindRose(int wDir, int x, int y){
  tft.drawSmoothCircle(x+24, y+24, 22, TFT_SILVER, DARKER_GREY);
  // Draw cecond hand
  float xp = 0.0, yp = 0.0;
  float xa = 0.0, ya = 0.0;
  getCoord(x+24, y+24, &xa, &ya, 6, 1.0f * wDir);
  tft.drawWedgeLine(x+24, y+24, xa, ya, 0.5, 4.5, TFT_RED);
  getCoord(x+24, y+24, &xp, &yp, 19, 1.0f * wDir);
  tft.drawWedgeLine(xa, ya, xp, yp,2.5, 2.5, TFT_RED);
}

void drawSymbol(int symbol) {
  int symbol_index = getSymbol[symbol];
  if(symbol_index > -1){
    int16_t rc = png.openFLASH((uint8_t *)fmisymbols[symbol_index], fmisizes[symbol_index], pngDraw);
    if (rc == PNG_SUCCESS) {
      tft.startWrite();
      rc = png.decode(NULL, 0);
      tft.endWrite();
    } 
    // png.close(); // not needed for memory->memory decode
  } else {
      tft.setTextFont(4);
      tft.setTextColor(TFT_RED);
      tft.print(" n/a");
      tft.setTextFont(6);
    } 
}





// =========================================================================
// Get coordinates of end of a line, pivot at x,y, length r, angle a
// =========================================================================
// Coordinates are returned to caller via the xp and yp pointers
#define DEG2RAD 0.0174532925
void getCoord(int16_t x, int16_t y, float *xp, float *yp, int16_t r, float a)
{
  float sx1 = cos( (a - 90) * DEG2RAD);
  float sy1 = sin( (a - 90) * DEG2RAD);
  *xp =  sx1 * r + x;
  *yp =  sy1 * r + y;
}

//=========================================v==========================================
//                                      pngDraw
//====================================================================================
// This next function will be called during decoding of the png file to
// render each image line to the TFT.  If you use a different TFT library
// you will need to adapt this function to suit.
// Callback function to draw pixels to the display
void pngDraw(PNGDRAW *pDraw) {
  uint16_t lineBuffer[MAX_IMAGE_WIDTH];
  
  png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_LITTLE_ENDIAN, 0xffffffff);
  tft.pushImage(xpos, ypos + pDraw->y, pDraw->iWidth, 1, lineBuffer);
}
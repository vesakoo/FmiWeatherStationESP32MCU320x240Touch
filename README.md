# 2.8inch_ESP32-2432S028R ESP32 MCU Weather station with timer

You can switch between timer and Weather station by touching the screen   


## Weather forecast 
This Application fetches a temperature forecast from given url and presents the result on TXT 240x240 panel
Original version is: https://github.com/vesakoo/FmiWeatherStationESP32S3  
## Timer 
Timer is based on TFT_sSPI examples expect it start counting time from zero.   



Forecast format:
```
{
  "name": "weather",
  "forecast": [
    {
      "hours": 1,
      "temp": -3,
      "wind": 5,
      "windDir": 47,
      "symbol": 7
    },
    ... 
```
Where hour is 01:00:00 given on your local time.

![alt text](https://github.com/vesakoo/FmiWeatherStationESP32MCU320x240/blob/main/images/display.jpg?raw=true)




### Web backend
Example backend solution:  https://github.com/vesakoo/fmiForecastEspoo  
Creates a weather.json file that one can place into webserver to share.   
Example renders weather forecast on Finland / Espoo - area fetching the data  
from  Finish institute of meteorology Open data interface:   
https://www.ilmatieteenlaitos.fi/avoin-data   


### Hardware
* ESP32 MCU 2.8 Inch Smart Display for Arduino LVGL WIFI Bluetooth Touch WROOM 240*320 Screen LCD TFT Module

https://www.aliexpress.com/item/1005004961285750.html   

http://pan.jczn1688.com/1/ESP32%20module   


### Requirements
* install libs   
https://github.com/Bodmer/TFT_eSPI   
Arduino JSON   
https://arduinojson.org/?utm_source=meta&utm_medium=library.properties   
PNGdec   
https://github.com/bitbank2/PNGdec   
TFT_Touch
https://github.com/Bodmer/TFT_Touch



* add SSID.h to with connection parameters for your WIFI router
SSID.h   
```
#define SECRET_SSID "your-secret"
#define SECRET_PASS "your-pass"
#define ACTION_TTL (unsigned long) 15000
#define AUTH      WIFLY_AUTH_WPA2_PSK
```

### Tools  

Tools folder contains a Python script to create symbols.h -file containing png-images in hex array format.
* Run script in same folder where your images are locted to create this file.
* Images on folder need to use numeric naming: 1.png,2.png,...
  (This numbers are stored in allocation table telling where is a pointer to a symbol given in index) 
* By default script handles images from 0-200.png
Example:
```
cp ./symbol_images/*.png ./tools/original
cd ./tools/original
python3 ../pngsymbolsToC.py
cp symbols.h ../../
```


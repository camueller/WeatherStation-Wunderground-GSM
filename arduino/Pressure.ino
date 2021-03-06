/*
 This Arduino sketch controls a personal weather station and uses a GSM shield to upload data to wunderground.com.
 For updates check https://github.com/camueller/WeatherStation-Wunderground-GSM
 */

//////////////
// Libraries
//////////////

#include <Wire.h>
#include <Adafruit_MPL3115A2.h>

//////////////
// Variables
//////////////
Adafruit_MPL3115A2 baro;

//////////////
// Set up
//////////////

void setupPressure() {
  baro = Adafruit_MPL3115A2();
}

//////////////
// Loop
//////////////

void loopPressure() {
  if (! baro.begin()) {
    Serial.println("Couldnt find sensor");
    return;
  }

  for(int i=0;i<5;i++) {
    if(pressure < 1) {
      pressure = baro.getPressure();
    }
    else {
      break;
    }
    delay(20);
  }
  
#ifdef INFO_WS
  Serial.print(F("Pressure="));
  Serial.print(pressure, 1);
  Serial.print(F("pa"));
#endif  
}


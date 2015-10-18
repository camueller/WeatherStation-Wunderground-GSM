/*
 This Arduino sketch controls a personal weather station and use an ethernet shield to upload data to wunderground.com.
 Details about the weather station can be found at http://www.avanux.de/space/Arduino/Wetterstation.
 */

//////////////
// Libraries
//////////////

#include <OneWire.h>
#include <DallasTemperature.h>

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(temperaturePin);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

void setupTemperature()
{
  sensors.begin(); // IC Default 9 bit. If you have troubles consider upping it 12. Ups the delay giving the IC more time to process the temperature measurement
}

void loopTemperature()
{ 
  sensors.requestTemperatures();
  temperature = sensors.getTempCByIndex(0);
#ifdef INFO_WS
  Serial.print(F("Temperature="));
  Serial.println(temperature);
#endif
}

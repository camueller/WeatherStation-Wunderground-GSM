/*
 This Arduino sketch controls a personal weather station and use an ethernet shield to upload data to wunderground.com.
 Details about the weather station can be found at http://www.avanux.de/space/Arduino/Wetterstation.
 */

//////////////
// Variables
//////////////

//////////////
// Set up
//////////////

void setupHumidity() {
}

//////////////
// Loop
//////////////

void loopHumidity() {
  int value = analogRead(humidityPin);
#ifdef DEBUG_WS  
    Serial.print(F("humidity (0-1023)="));
    Serial.println(value);
#endif
  // from data sheet
  humidity = ((value/1023.0-0.1515)/0.00636)/(1.0546-0.00216*temperature);
  dewpoint = dewPointFast(temperature, humidity);
  
#ifdef INFO_WS
  Serial.print(F("Humidity="));
  Serial.print(humidity, 1);
  Serial.print(F("%, Dewpoint="));
  Serial.print(dewpoint, 1);
  Serial.println(F("*C"));
#endif  
}

// delta max = 0.6544 wrt dewPoint()
// 5x faster than dewPoint()
// reference: http://en.wikipedia.org/wiki/Dew_point
double dewPointFast(double celsius, double humidity) {
  double a = 17.271;
  double b = 237.7;
  double temp = (a * celsius) / (b + celsius) + log(humidity/100);
  double Td = (b * temp) / (a - temp);
  return Td;
}


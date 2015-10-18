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

void setupSolarRadiation() {
}

//////////////
// Loop
//////////////

void loopSolarRadiation() {
  int solarRadiationRead = analogRead(solarRadiationPin);
  solarRadiation = map(solarRadiationRead);
  
#ifdef INFO_WS
  Serial.print(F("Solar radiation="));
  Serial.println(solarRadiation);
#endif
}

int map(int value)  { 
  // Correlation to other stations reporting solar radiation
  //
  //          IHESSENA6   IHESSENS5
  //          analogRead  W/m2
  //          150 Ohm
  // 28.07.14             880
  // 07.02.14 422         358
  //
  return value;
}

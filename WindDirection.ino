/*
 This Arduino sketch controls a personal weather station and uses a GSM shield to upload data to wunderground.com.
 For updates check https://github.com/camueller/GSM-GPRS-GPS-Shield
 */

//////////////
// Variables
//////////////

const long WIND_DIRECTION_AVG_INTERVAL_SECONDS = 120;
// voltage (mV) over wind vane every 22.5 degree according to wind vane vendor specification
const int windDirectionVoltageForDirection[] = {
  3840, 1980, 2250, 410, 450, 320, 900, 620, 1400, 1190, 3080, 2930, 4620, 4040, 4780, 3430};
unsigned long windDirectionAvgValues;
unsigned long windDirectionAvgValueSum;
unsigned long windDirectionAvgIntervalBegin;

//////////////
// Set up
//////////////

void setupWindDirection()
{
}

//////////////
// Loop
//////////////

void loopWindDirection(unsigned long now)
{ 
    int windDirectionRead = analogRead(windDirectionPin);
    int windDirectionVoltage = map(windDirectionRead, 0, 1023, 0, 5000);
#ifdef DEBUG_WS  
    Serial.print(F("wind direction: read="));
    Serial.print(windDirectionRead);
    Serial.print(F(" voltage="));
    Serial.println(windDirectionVoltage);
#endif
    windDirection = getWindDirection(windDirectionVoltage);
#ifdef INFO_WS
    Serial.print(F("Wind direction="));
    Serial.println(windDirection);
#endif
    
    // calculate average
    windDirectionAvgValues++;
    windDirectionAvgValueSum += windDirection;
  
    if(now - windDirectionAvgIntervalBegin > WIND_DIRECTION_AVG_INTERVAL_SECONDS * 1000 || windDirectionAvgIntervalBegin > now) {
      windDirectionAvg = windDirectionAvgValueSum / windDirectionAvgValues;

#ifdef INFO_WS
      Serial.print(F("Wind direction ("));
      Serial.print(WIND_DIRECTION_AVG_INTERVAL_SECONDS);
      Serial.print(F(" s): "));
      Serial.println(windDirectionAvg);
#endif
      
      windDirectionAvgIntervalBegin = now;
      windDirectionAvgValues = 0;
      windDirectionAvgValueSum = 0;
    }
    
}

int getWindDirection(int measuredVoltage) {
  int windDirectionIndex = -1;
  // measuredVoltage will not exactly match sepcified voltages
  // therefore we use an increasing range around each voltage value until we find a match
  for(int range=10; range<1000; range+=10) {
    for(int i=0; i<16; i++) {
      int directionVoltage = windDirectionVoltageForDirection[i];
      if(directionVoltage - range < measuredVoltage && measuredVoltage < directionVoltage + range) {
        windDirectionIndex = i;
        break;
      }
    }
    if(windDirectionIndex > -1) {
      break;
    }
  }  
  return windDirectionIndex * 22.5;
}


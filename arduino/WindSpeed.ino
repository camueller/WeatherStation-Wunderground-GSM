/*
 This Arduino sketch controls a personal weather station and uses a GSM shield to upload data to wunderground.com.
 For updates check https://github.com/camueller/GSM-GPRS-GPS-Shield
 */

//////////////
// Variables
//////////////

// calculate wind speed across multiple seconds; otherwise speed can only be a multiple of 2.4 kph
const int WIND_SPEED_CALC_INTERVAL_SECONDS = 3;
const long WIND_SPEED_AVG_INTERVAL_SECONDS = 120;
const long WIND_SPEED_GUST_INTERVAL_SECONDS = 600;
volatile uint16_t windSpeedSignalCount;
unsigned long  windCalcIntervalBegin;
unsigned long  windSpeedAvgIntervalBegin;
         float windSpeedAvgValues;
         float windSpeedAvgValueSum;
unsigned long  windSpeedGustIntervalBegin;

//////////////
// Set up
//////////////

void setupWindSpeed(unsigned long now)
{
  attachInterrupt(windSpeedInterrupt, windSignal, RISING);
  windSpeed = 0.0;
  windSpeedAvg = 0.0;
  windSpeedGust = 0.0;
  initWindSpeedSignalCount();
  windCalcIntervalBegin = now;
  windSpeedAvgIntervalBegin = now;
  windSpeedGustIntervalBegin = now;
}

void initWindSpeedSignalCount() {
  windSpeedSignalCount = 0;
}

//////////////
// Loop
//////////////

void loopWindSpeed(unsigned long now)
{
  // calculate wind speed  
  if(now - windCalcIntervalBegin > 1000 * WIND_SPEED_CALC_INTERVAL_SECONDS || windCalcIntervalBegin > now) {
    windSpeed = windSpeedSignalCount * 2.4 / WIND_SPEED_CALC_INTERVAL_SECONDS;
#ifdef DEBUG_WS  
    Serial.print(F("windSpeedSignalCount="));
    Serial.println(windSpeedSignalCount);
#endif
#ifdef INFO_WS
    Serial.print(F("wind speed "));
    Serial.print(windSpeed);
    Serial.println(F(" kph"));
#endif  
    windSpeedSignalCount=0;  
    windCalcIntervalBegin = now;
    
    // calculate average
    windSpeedAvgValues++;
    windSpeedAvgValueSum += windSpeed;
  
    if(now - windSpeedAvgIntervalBegin > WIND_SPEED_AVG_INTERVAL_SECONDS * 1000) {
      windSpeedAvg = windSpeedAvgValueSum / windSpeedAvgValues;

#ifdef DEBUG_WS  
      Serial.print(F("windSpeedAvgValueSum="));
      Serial.print(windSpeedAvgValueSum);
      Serial.print(F(" windSpeedAvgValues="));
      Serial.println(windSpeedAvgValues);
#endif
#ifdef INFO_WS
      Serial.print(F("Wind avg("));
      Serial.print(WIND_SPEED_AVG_INTERVAL_SECONDS);
      Serial.print(F("s): "));
      Serial.print(windSpeedAvg);
      Serial.println(F(" kph"));
#endif      
      windSpeedAvgIntervalBegin = now;
      windSpeedAvgValues = 0;
      windSpeedAvgValueSum = 0;
    }
    
    if(now - windSpeedGustIntervalBegin > WIND_SPEED_GUST_INTERVAL_SECONDS * 1000 || windSpeedGustIntervalBegin > now) {
#ifdef INFO_WS
      Serial.print(F("Wind gusts ("));
      Serial.print(WIND_SPEED_GUST_INTERVAL_SECONDS);
      Serial.print(F("s): "));
      Serial.print(windSpeedGust);
      Serial.println(F(" kph"));
#endif      
      windSpeedGustIntervalBegin = now;
      windSpeedGust = 0.0;
    }
    if(windSpeed > windSpeedGust) {
      windSpeedGust = windSpeed;
      windGustDirection = windDirection;
    }
  }
}

//////////////
// Functions
//////////////

void windSignal()
{
  windSpeedSignalCount++;
}

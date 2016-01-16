/*
 This Arduino sketch controls a personal weather station and uses a GSM shield to upload data to wunderground.com.
 For updates check https://github.com/camueller/WeatherStation-Wunderground-GSM
 */

//////////////
// Variables
//////////////

const unsigned long RAIN_INTERVAL_SECONDS                   = 300;
const          int  RAIN_INTERVALS                          = 12; // 12 * 5min = 60 min
unsigned       int  rainSignalCountInterval[RAIN_INTERVALS];
unsigned       int  rainSignalCountIntervalIndex;
volatile       int  rainSignalCount;
unsigned       long rainIntervalBegin;
               boolean rainTodayReset = false;

//////////////
// Set up
//////////////

void setupRain(unsigned long now) {
  attachInterrupt(rainInterrupt, rainSignal, RISING);
  rainSignalCount = 0;
  rainSignalCountIntervalIndex = 0;
  memset(rainSignalCountInterval, 0, sizeof(rainSignalCountInterval));
  rainIntervalBegin = now;
  rainToday = 0;
}

//////////////
// Loop
//////////////

void loopRain(unsigned long now) {
#ifdef DEBUG_WS  
    Serial.print(F("rainSignalCount="));
    Serial.print(rainSignalCount);
    Serial.print(F(" rainSignalCountIntervalIndex="));
    Serial.println(rainSignalCountIntervalIndex);
#endif
  // calculate rain in interval
  boolean intervalIndexReset = false;
  if(now - rainIntervalBegin > 1000 * RAIN_INTERVAL_SECONDS || rainIntervalBegin > now) {
    rainSignalCountInterval[rainSignalCountIntervalIndex] = rainSignalCount;
    rainSignalCount = 0;
    rainIntervalBegin = now;
    
#ifdef DEBUG_WS  
    for(int i=0;i<RAIN_INTERVALS;i++) {
      Serial.print(F("rainSignalCountInterval["));
      Serial.print(i);
      Serial.print(F("]="));
      Serial.println(rainSignalCountInterval[i]);
    }
#endif

    rainSignalCountIntervalIndex += 1;
    if(rainSignalCountIntervalIndex >= RAIN_INTERVALS) {
      rainSignalCountIntervalIndex = 0;
      intervalIndexReset = true;
    }
  }  

  // calculate rain total of all intervals
  int rainSignalCountTotal = 0;
  for(int i=0;i<RAIN_INTERVALS;i++) {
    rainSignalCountTotal += rainSignalCountInterval[i];
  }
  rainLastHour = rainSignalCountTotal * 0.2794;

  // calculate rainfall today  
  if(hourOfDay == 0) {
    // during summer time this is 1AM rather than midnight since time is GMT
    if(! rainTodayReset) {
      // reset once per day
      rainTodayReset = true;
      rainToday = rainLastHour; 
    }
  }
  else {
    // it's later than 1AM now - allow for reset the next time a new day begins
    rainTodayReset = false;
  }
  if(intervalIndexReset) {
    rainToday += rainLastHour;
  }

#ifdef INFO_WS
  Serial.print(F("Rainfall ("));
  Serial.print(RAIN_INTERVAL_SECONDS * RAIN_INTERVALS);
  Serial.print(F("s): "));
  Serial.print(rainLastHour);
  Serial.println(F(" mm"));

  Serial.print(F("Rainfall today: "));
  Serial.print(rainToday);
  Serial.println(F(" mm"));
#endif
}

//////////////
// Functions
//////////////

void rainSignal() {
  rainSignalCount++;
}


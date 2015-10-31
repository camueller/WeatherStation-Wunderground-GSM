/*
 This Arduino sketch controls a personal weather station and uses a GSM shield to upload data to wunderground.com.
 For updates check https://github.com/camueller/GSM-GPRS-GPS-Shield
 */

//////////////
// Libraries
//////////////

#include <avr/wdt.h>

//////////////
// Variables
//////////////

//#define INFO_WS 1    // INFO is already defined in LOG.h of GSM GPRS shield
//#define DEBUG_WS 1   // DEBUG is already defined in LOG.h of GSM GPRS shield
//#define DEBUG_UPLOAD 1
#define UPLOAD 1

#define ENABLE_TEMPERATURE 1
#define ENABLE_WIND_SPEED 1
#define ENABLE_WIND_DIRECTION 1
#define ENABLE_RAIN 1
#define ENABLE_SOLAR_RADIATION 1
#define ENABLE_HUMIDITY 1
// #define ENABLE_PRESSURE 1

#define WDT_COUNTER_MAX 40  // 40=320s : Number of times of ISR(WDT_vect) to autoreset the board. I will autoreset the board after 8 secondes x counterMax
volatile int wdtCounter; 

unsigned const long MAX_LONG = 4294967295;
unsigned const int ANALOG_READINGS = 100;
unsigned int hourOfDay = -1;

// results
unsigned int   humidity;
float          temperature;
float          dewpoint;
         float windSpeed;
         float windSpeedAvg;
         float windSpeedGust;
unsigned int   windDirection;
unsigned int   windDirectionAvg;
unsigned int   windGustDirection;
         float rainLastHour;
         float rainToday;
unsigned int   solarRadiation;
         float pressure;

//////////////
// Pins set up
//////////////

// analog input
unsigned const int unusedAnalogPin   = A0;
unsigned const int windDirectionPin  = A2;
unsigned const int solarRadiationPin = A3;
unsigned const int humidityPin       = A4;

// digital
unsigned const int windSpeedInterrupt= 0; // D2
unsigned const int rainInterrupt     = 1; // D3
unsigned const int temperaturePin    = 7;
unsigned const int gsmPin            = 6;

//////////////
// Watchdog
// http://www.seeedstudio.com/recipe/279-long-time-auto-reset-feature-for-your-arduino.html
//////////////

ISR (WDT_vect)
{
        wdtCounter += 1;
        if (wdtCounter < WDT_COUNTER_MAX - 1) {
            wdt_reset(); // Reset timer, still in interrupt mode       
                         // Next time watchdogtimer complete the cicle, it will generate antoher ISR interrupt
        } else {
            MCUSR = 0;
            WDTCSR |= 0b00011000;    //WDCE y WDE = 1 --> config mode
            WDTCSR = 0b00001000 | 0b100001;    //clear WDIE (interrupt mode disabled), set WDE (reset mode enabled) and set interval to 8 seconds
                                               //Next time watchdog timer complete the cicly will reset the IC
        }
}

void wdt_long_enable()
{
    wdtCounter = 0;
    cli();    //disabled ints
    MCUSR = 0;   //clear reset status
    WDTCSR |= 0b00011000;    //WDCE y WDE = 1 --> config mode
    WDTCSR = 0b01000000 | 0b100001;    //set WDIE (interrupt mode enabled), clear WDE (reset mode disabled) and set interval to 8 seconds
    sei();   //enable ints
}

void wdt_long_disable()
{
    wdt_disable();
}

//////////////
// Set up
//////////////
void setup() {
#ifdef DEBUG_WS
  setupLogging();
#elif INFO_WS
  setupLogging();
#endif

  wdt_disable();
  
  unsigned long now = currentMillis();
#ifdef ENABLE_TEMPERATURE
  setupTemperature();
#endif
#ifdef ENABLE_WIND_SPEED
  setupWindSpeed(now);
#endif
#ifdef ENABLE_WIND_DIRECTION
  setupWindDirection();
#endif
#ifdef ENABLE_RAIN
  setupRain(now);
#endif
#ifdef ENABLE_SOLAR_RADIATION
  setupSolarRadiation();
#endif
#ifdef ENABLE_HUMIDITY
  setupHumidity();
#endif
#ifdef ENABLE_PRESSURE
  setupPressure();
#endif

#ifdef UPLOAD
  setupUpload(now);
#endif
}

void setupLogging() {
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
}

//////////////
// Loop
//////////////

void loop() {

  wdt_long_enable();
  
  unsigned long now = currentMillis();
#ifdef INFO_WS
  Serial.print(F("\nnow="));Serial.print(now);Serial.print(F("\tfree="));Serial.println(freeRam());
#endif

#ifdef ENABLE_TEMPERATURE
  loopTemperature();
#endif
#ifdef ENABLE_WIND_SPEED
  loopWindSpeed(now);
#endif
#ifdef ENABLE_WIND_DIRECTION
  loopWindDirection(now);
#endif
#ifdef ENABLE_RAIN
  loopRain(now);
#endif
#ifdef ENABLE_SOLAR_RADIATION
  loopSolarRadiation();
#endif
#ifdef ENABLE_HUMIDITY
  loopHumidity();
#endif
#ifdef ENABLE_PRESSURE
  loopPressure();
#endif

#ifdef UPLOAD
  loopUpload(now);
#endif

  delay(1000);

  wdt_long_disable();
}

//////////////
// Functions
//////////////

/*
 * The sensors are connected to the Arduino with long wires that act as antennas and pick up noise. This caused the analog readings to fluctuate.
 * A first step towards repressing this noise could be to do 100 A/D conversions and taking the average. While this is a very straight forward approach, 
 * there are better options.
 * A median filter is much better in reducing the impact of noise than an averaging filter. It works like this: you take 100 measurements,
 * sort them from small to large and take the middle one. A median filter is much less influenced by outliers, since they end up at the beginning or the end.
 * A mode filter is a combination of a median filter and an averaging filter: you sort the values and take the average of the ones in the middle. 
 * In my case I take the average of the 10 center values.
 * 
 * Source: http://www.elcojacobs.com/eleminating-noise-from-sensor-readings-on-arduino-with-digital-filtering/
 * 
 * Note: this method takes 18 ms for 100 readings additional to the initial delay after reading the unusedAnalogPin
 */
int analogReadSmoothed(int pin){
  analogRead(unusedAnalogPin); // dummy read to discharge ADC
  delay(20); // wait a little
  // read multiple values and sort them to take the mode
  int sortedValues[ANALOG_READINGS];
  for(int i=0;i<ANALOG_READINGS;i++){
    int value = analogRead(pin);
    int j;
    if(value<sortedValues[0] || i==0){
       j=0; //insert at first position
    }
    else {
      for(j=1;j<i;j++) {
         if(sortedValues[j-1]<=value && sortedValues[j]>=value){
           // j is insert position
           break;
         }
      }
    }
    for(int k=i;k>j;k--){
      // move all values higher than current reading up one position
      sortedValues[k]=sortedValues[k-1];
    }
    sortedValues[j]=value; //insert current reading
  }
  //return scaled mode of 10 values
  float returnval = 0;
  for(int i=ANALOG_READINGS/2-5;i<(ANALOG_READINGS/2+5);i++){
    returnval +=sortedValues[i];
  }
  return returnval/10;
}

long currentMillis() {
#ifdef DEBUG_WS  
  return MAX_LONG - 10000 + millis();
#endif  
  return millis();
}

int freeRam () 
{
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}


 /*
 This Arduino sketch controls a personal weather station and use an ethernet shield to upload data to wunderground.com.
 Details about the weather station can be found at http://www.avanux.de/space/Arduino/Wetterstation.
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

#define WDT_COUNTER_MAX 40  // 40=320s : Number of times of ISR(WDT_vect) to autoreset the board. I will autoreset the board after 8 secondes x counterMax
volatile int wdtCounter; 

unsigned const long MAX_LONG = 4294967295;
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

//////////////
// Pins set up
//////////////

// analog input
unsigned const int windDirectionPin  = A2;
unsigned const int solarRadiationPin = A3;
unsigned const int humidityPin       = A4;

// digital
unsigned const int rainInterrupt     = 0; // D3
unsigned const int windSpeedInterrupt= 1; // D2
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
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
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

#ifdef UPLOAD
  setupUpload(now);
#endif
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

#ifdef UPLOAD
  loopUpload(now);
#endif

  delay(1000);

  wdt_long_disable();
}

//////////////
// Functions
//////////////

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


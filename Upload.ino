/*
 This Arduino sketch controls a personal weather station and uses a GSM shield to upload data to wunderground.com.
 For updates check https://github.com/camueller/GSM-GPRS-GPS-Shield
 */

//////////////
// Libraries
//////////////

#include "SIM900.h"
#include <SoftwareSerial.h>
#include "inetGSM.h"
#include <avr/pgmspace.h>

//////////////
// Variables
//////////////

const          char    GET[] PROGMEM             = "GET ";
const          char    PATH1[] PROGMEM           = "/weatherstation";
const          char    PATH2[] PROGMEM           = "/updateweatherstation.php?";
// The line below containing the correct value is contained in a Credentials.ino
//const          char    ID[] PROGMEM              = "ID=IABCDEF";
// The line below containing the correct value is contained in a Credentials.ino
//const          char    PASSWORD[] PROGMEM        = "&PASSWORD=AbcDef";
const          char    DATE[] PROGMEM            = "&dateutc=now";
const          char    TEMP[] PROGMEM            = "&tempf=";
const          char    HUMIDITY[] PROGMEM        = "&humidity=";
const          char    DEWPT[] PROGMEM           = "&dewptf=";
const          char    WINDDIR[] PROGMEM         = "&winddir=";
const          char    WINDDIR_AVG2M[] PROGMEM   = "&winddir_avg2m=";
const          char    WINDGUSTDIR[] PROGMEM     = "&windgustdir=";
const          char    WINDSPEED[] PROGMEM       = "&windspeedmph=";
const          char    WINDSPEED_AVG2M[] PROGMEM = "&windspdmph_avg2m=";
const          char    WINDGUST[] PROGMEM        = "&windgustmph=";
const          char    WINDGUST_10M[] PROGMEM    = "&windgustmph_10m=";
const          char    BAROM[] PROGMEM           = "&baromin=";
const          char    RAIN[] PROGMEM            = "&rainin=";
const          char    RAIN_DAILY[] PROGMEM      = "&dailyrainin=";
const          char    SOLARRADIATION[] PROGMEM  = "&solarradiation=";
const          char    PRESSURE[] PROGMEM        = "&baromin=";
const          char    UPDATERAW[] PROGMEM       = "&action=updateraw";
const          char    HTTP_HOST[] PROGMEM       = " HTTP/1.0\nHost: ";
const          char    SERVER[] PROGMEM          = "weatherstation.wunderground.com";
const          char    USER_AGENT[] PROGMEM      = "\nUser-Agent: Arduino\n\n";

const          char* const path[] PROGMEM = { GET,             // 0
                                              PATH1,           // 1
                                              PATH2,           // 2
                                              ID,              // 3 
                                              PASSWORD,        // 4
                                              DATE,            // 5
                                              TEMP,            // 6
                                              HUMIDITY,        // 7
                                              DEWPT,           // 8
                                              WINDDIR,         // 9 
                                              WINDDIR_AVG2M,   // 10
                                              WINDGUSTDIR,     // 11
                                              WINDSPEED,       // 12
                                              WINDSPEED_AVG2M, // 13
                                              WINDGUST,        // 14
                                              WINDGUST_10M,    // 15
                                              BAROM,           // 16
                                              RAIN,            // 17
                                              RAIN_DAILY,      // 18
                                              SOLARRADIATION,  // 19
                                              PRESSURE,        // 20
                                              UPDATERAW,       // 21
                                              HTTP_HOST,       // 22
                                              SERVER,          // 23
                                              USER_AGENT       // 24
                                            };

const unsigned long    UPLOAD_INTERVAL_SECONDS = 300; // muss 300 sein
const unsigned int     GMT_OFFSET              = 1;
unsigned       long    upoadIntervalBegin;
InetGSM inet;

//////////////
// Set up
//////////////

void setupUpload(unsigned long now) {
#ifdef DEBUG_WS  
    Serial.println(F("setupUpload"));
#endif
  upoadIntervalBegin = now;
  turnOffGsm();
}

//////////////
// Loop
//////////////

void loopUpload(unsigned long now) {
  if(now - upoadIntervalBegin > 1000 * UPLOAD_INTERVAL_SECONDS) {
//  if(1 == 1) {
    turnOnGsm();
    
    //Start configuration of shield with baudrate.
    //For http uses is raccomanded to use 4800 or slower.
    boolean started;
    if (gsm.begin(9600)) {
      Serial.println(F("\nstatus=READY"));
      started=true;
    }
    else Serial.println(F("\nstatus=IDLE"));
  
    if(started){
      //GPRS attach, put in order APN, username and password.
      //If no needed auth let them blank.
      int attached = inet.attachGPRS("internet.telekom", "tm", "tm");
#ifdef DEBUG_UPLOAD
      if (attached)
        Serial.println(F("status=ATTACHED"));
      else
        Serial.println(F("status=ERROR"));
#endif
      delay(1000);

      //Read IP address.
      gsm.SimpleWriteln("AT+CIFSR");
      delay(5000);
      //Read until serial buffer is empty.
      gsm.WhileSimpleRead();

      const int dummyLen = 0;
      char dummy[dummyLen];
      if(httpGET(dummy, dummyLen) >= 0) {
#ifdef DEBUG_UPLOAD
        Serial.println(F("Analyzing response"));
#endif  
        
        // HTTP/1.0 200 OK
        // Content-Type: text/html
        // Date: Sat, 25 Apr 2015 06:00:00 GMT
        // Content-Length: 8
        // Connection: close
        //
        // success
        
        byte status = gsm.WaitResp(10000, 2000); 
        if (status == RX_FINISHED) {
#ifdef DEBUG_UPLOAD
          Serial.println(F("Response availabe in comm buffer"));
#endif
          char searchFor[4 + 1];
          for(int i=0;i<24;i++) {
            sprintf(searchFor," %02d:",i);
            if(gsm.IsStringReceived(searchFor)) {
              hourOfDay = i + GMT_OFFSET;
            }    
          }
        }
#ifdef DEBUG_UPLOAD
        Serial.print(F("hourOfDay="));Serial.println(hourOfDay);
#endif  
      }
    }
      
    turnOffGsm();
    
    upoadIntervalBegin = now;
  }

  // uploading may take up to 40 seconds, therefore we have to ignore
  // the count increase during that time; otherwise we will see very 
  // high wind speed because of that wrong count increase
  initWindSpeedSignalCount();
}

void turnOnGsm() {
#ifdef DEBUG_UPLOAD
      Serial.println(F("Turn on GSM"));
#endif	
  digitalWrite(gsmPin, HIGH);
  delay(2500);
  digitalWrite(gsmPin, LOW);
  delay(2500);
}

int turnOffGsm() {
#ifdef DEBUG_UPLOAD
      Serial.println(F("Turn off GSM"));
#endif	
  gsm.SimpleWriteln(F("AT+CPOWD=1"));
//  delay(2500);
//  digitalWrite(gsmPin, LOW);
//  delay(2000);
//  digitalWrite(gsmPin, HIGH);
}  

// taken from inetGSM.cpp in order to be able to read URL segments from PROGMEM
// rather than having to construct the whole URL upfront (requires too much memory)
int httpGET(char* result, int resultlength)
{
  boolean connected=false;
  int n_of_at=0;
  int length_write;
  char end_c[2];
  end_c[0]=0x1a;
  end_c[1]='\0';

  char buffer[40];
  char *ptrBuffer = buffer;

  // extablish TCP connection to server
  strcpy_P(buffer, (char*)pgm_read_word(&(path[23])));
  while(n_of_at<3){
	  if(!inet.connectTCP(ptrBuffer, 80)){
	  	#ifdef DEBUG_UPLOAD
			Serial.print(F("Not connected="));Serial.println(ptrBuffer);
		#endif	
	    	n_of_at++;
	  }
	  else{
		connected=true;
		n_of_at=3;
	}
  }

  if(!connected) {
  #ifdef DEBUG_UPLOAD
    Serial.print(F("Not connected="));Serial.println(ptrBuffer);
  #endif	
    return -1;
  }
  #ifdef DEBUG_UPLOAD
    Serial.print(F("Connected="));Serial.println(ptrBuffer);
  #endif	
  
  for (int i = 0; i < 6; i++)
  {
    strcpy_P(buffer, (char*)pgm_read_word(&(path[i])));
    composeHttpGET(ptrBuffer);
  }
  
#ifdef ENABLE_TEMPERATURE
  strcpy_P(buffer, (char*)pgm_read_word(&(path[6])));
  composeHttpGET(ptrBuffer);
  celsius2fahrenheit(temperature, buffer);
  composeHttpGET(ptrBuffer);
#endif

#ifdef ENABLE_HUMIDITY
  strcpy_P(buffer, (char*)pgm_read_word(&(path[7])));
  composeHttpGET(ptrBuffer);
  itoa(humidity, buffer, 10);
  composeHttpGET(ptrBuffer);
      
  strcpy_P(buffer, (char*)pgm_read_word(&(path[8])));
  composeHttpGET(ptrBuffer);
  celsius2fahrenheit(dewpoint, buffer);
  composeHttpGET(ptrBuffer);
#endif

#ifdef ENABLE_WIND_DIRECTION
  strcpy_P(buffer, (char*)pgm_read_word(&(path[9])));
  composeHttpGET(ptrBuffer);
  itoa(windDirection, buffer, 10);
  composeHttpGET(ptrBuffer);
      
  strcpy_P(buffer, (char*)pgm_read_word(&(path[10])));
  composeHttpGET(ptrBuffer);
  itoa(windDirectionAvg, buffer, 10);
  composeHttpGET(ptrBuffer);
      
  strcpy_P(buffer, (char*)pgm_read_word(&(path[11])));
  composeHttpGET(ptrBuffer);
  itoa(windGustDirection, buffer, 10);
  composeHttpGET(ptrBuffer);
#endif

#ifdef ENABLE_WIND_SPEED
  strcpy_P(buffer, (char*)pgm_read_word(&(path[12])));
  composeHttpGET(ptrBuffer);
  kph2mph(windSpeed, buffer);
  composeHttpGET(ptrBuffer);
      
  strcpy_P(buffer, (char*)pgm_read_word(&(path[13])));
  composeHttpGET(ptrBuffer);
  kph2mph(windSpeedAvg, buffer);
  composeHttpGET(ptrBuffer);
      
  strcpy_P(buffer, (char*)pgm_read_word(&(path[14])));
  composeHttpGET(ptrBuffer);
  kph2mph(windSpeedGust, buffer);
  composeHttpGET(ptrBuffer);
      
  strcpy_P(buffer, (char*)pgm_read_word(&(path[15])));
  composeHttpGET(ptrBuffer);
  kph2mph(windSpeedGust, buffer);
  composeHttpGET(ptrBuffer);
#endif

#ifdef ENABLE_PRESSURE
  strcpy_P(buffer, (char*)pgm_read_word(&(path[16])));
  composeHttpGET(ptrBuffer);
  pa2inHg(pressure, buffer);
  composeHttpGET(ptrBuffer);
#endif
#ifdef ENABLE_RAIN
  strcpy_P(buffer, (char*)pgm_read_word(&(path[17])));
  composeHttpGET(ptrBuffer);
  mm2in(rainLastHour, buffer);
  composeHttpGET(ptrBuffer);

  strcpy_P(buffer, (char*)pgm_read_word(&(path[18])));
  composeHttpGET(ptrBuffer);
  mm2in(rainToday, buffer);
  composeHttpGET(ptrBuffer);
#endif
#ifdef ENABLE_SOLAR_RADIATION
  strcpy_P(buffer, (char*)pgm_read_word(&(path[19])));
  composeHttpGET(ptrBuffer);
  itoa(solarRadiation, buffer, 10);
  composeHttpGET(ptrBuffer);
#endif
#ifdef ENABLE_PRESSURE
  strcpy_P(buffer, (char*)pgm_read_word(&(path[20])));
  composeHttpGET(ptrBuffer);
  pa2inHg(pressure, buffer);
  composeHttpGET(ptrBuffer);
#endif

  for (int i = 21; i <= 24; i++)
  {
    strcpy_P(buffer, (char*)pgm_read_word(&(path[i])));
    composeHttpGET(ptrBuffer);
  }
  
  gsm.SimpleWrite(end_c);

  switch(gsm.WaitResp(10000, 10, "SEND OK")){
	case RX_TMOUT_ERR: 
		return 0;
	break;
	case RX_FINISHED_STR_NOT_RECV: 
		return 0; 
	break;
  }
  
  delay(50);
#ifdef DEBUG_UPLOAD
  Serial.println(F("HTTP GET sent"));
#endif	
  int res=gsm.read(result, resultlength);
#ifdef DEBUG_UPLOAD
  Serial.println(F("HTTP response received"));
#endif	

  return res;
}

int composeHttpGET(char* part) {
  gsm.SimpleWrite(part);
#ifdef DEBUG_UPLOAD  
  Serial.print(part);
#endif
}

char* kph2mph(float kph, char *buffer) {
  return ftoa(buffer, kph * 0.621371192, 1);
}

char* celsius2fahrenheit(float celsius, char *buffer) {
  return ftoa(buffer, celsius * 1.8 + 32, 1);
}

char* pa2inHg(float pa, char *buffer) {
  // http://www.engineeringtoolbox.com/pressure-units-converter-d_569.html
  return ftoa(buffer, pa * 0.000296, 1);
}

char* mm2in(float mm, char *buffer) {
  return ftoa(buffer, mm / 25.4, 1);
}

char *ftoa(char *a, double f, int precision)
{
 // http://forum.arduino.cc/index.php?topic=44262.0
 long p[] = {0,10,100,1000,10000,100000,1000000,10000000,100000000};
 
 char *ret = a;
 long heiltal = (long)f;
 itoa(heiltal, a, 10);
 while (*a != '\0') a++;
 *a++ = '.';
 long desimal = abs((long)((f - heiltal) * p[precision]));
 itoa(desimal, a, 10);
 return ret;
}


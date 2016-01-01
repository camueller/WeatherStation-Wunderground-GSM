[Deutsche Seite](README_DE.md)

GSM based weather station for the Weather Underground Network
======
I'm running this weather station on our paramotor airfield.

```html
<object width="290" height="130"><param name="movie" value="http://www.wunderground.com/swf/pws_mini_rf_nc.swf?station=IHESSENN7&freq=&units=english&lang=EN" /><embed src="http://www.wunderground.com/swf/pws_mini_rf_nc.swf?station=IHESSENN7&freq=&units=english&lang=EN" type="application/x-shockwave-flash" width="290" height="130" /></object>
```

### Hardware:
- [Arduino Leonardo](https://www.arduino.cc/en/Main/ArduinoBoardLeonardo)
- [EFCom_GPRS/GSM_Shield](http://www.elecfreaks.com/wiki/index.php?title=EFCom_GPRS/GSM_Shield)

### Sensors:
- wind speed (from WH1080)
- wind direction (from WH1080)
- temperature (Maxim DS18B20)
- humidity (Honeywell HIH-5031)
- rain (from WH1080)
- solar radiation (8x BPW34)

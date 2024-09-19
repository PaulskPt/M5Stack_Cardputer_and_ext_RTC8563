#  M5Stack Cardputer with external RTC unit

Preparations:

 Add your WiFi SSID, PASSWORD, TIMEZONE and PRIMARY NTP SERVER ADDRESS to file secret.h

What this sketch does:

 This arduino sketch provides two pages with date, time and (in the case of ESP32 LOCAL page also daylight saving status ("dst" if inside the daylight saving period of the year)).


 Page 1 shows: ESP32 LOCAL date, time and dst details;
 
 Page 2 shows: external RTC UTC date and time details.

Howto switch page:

 Switching page by pressing the ```<p>``` key on the Cardputer keyboard.



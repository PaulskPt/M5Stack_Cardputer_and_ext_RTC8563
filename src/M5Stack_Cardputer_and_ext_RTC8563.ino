
/*
  This sketch has changes made by @PaulskPt (Github) from the original repo: 
  https://github.com/m5stack/M5Unified/blob/master/examples/Basic/Rtc/Rtc.ino

  This sketch provides two display pages:
  1) ESP32 LOC (Localtime)  (default);
  2) RTC UTC;
  Page switching by pressing keyboard key <p>.
  When displaying localtime, the daylightsaving time indicator "Y" or "N" is shown to the right of the time.

*/
#if defined ( ARDUINO )

 #define WIFI_SSID     SECRET_SSID  // "Vodafone-8D96F1"
 #define WIFI_PASSWORD SECRET_PASS  // "z2pTqpFMMX6jXdv3"
 #define NTP_TIMEZONE  "utc+1"
 #define NTP_SERVER1   "2.pt.pool.ntp.org"
 #define NTP_SERVER2   "1.pool.ntp.org"
 #define NTP_SERVER3   "2.pool.ntp.org"

 #include <WiFi.h>
 #include "dst_PRT.h"
 #include <TimeLib.h>
 #include <time.h>

 // See: https://github.com/m5stack/M5Unified/blob/master/src/utility/RTC8563_Class.h and RTC8563_Class.hpp

bool use_local_time = true; // for the ESP32 internal clock 

// Different versions of the framework have different SNTP header file names and availability.
 #if __has_include (<esp_sntp.h>)
  #include <esp_sntp.h>
  #define SNTP_ENABLED 1
 #elif __has_include (<sntp.h>)
  #include <sntp.h>
  #define SNTP_ENABLED 1
 #endif

#endif

#include "secret.h"
#include <M5Cardputer.h>
#include <Wire.h>
#include <M5GFX.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#ifndef SNTP_ENABLED
#define SNTP_ENABLED 0
#endif

#include <M5Unified.h>

String TITLE =  "Unified RTC example";
int title_h = 0; // title horizontal space from left side display
int dw = 0;  // display width
int dh = 0;  // display height
unsigned int colour = 0;
int hori[] = {0, 80, 120, 200};
int vert[] = {20, 45, 70, 95, 120};
unsigned long t_start = millis();
static constexpr const char* const wd[7] = {"Sunday","Monday","Tuesday","Wednesday","Thursday","Fridat","Saturday"};

void load_dst_dict(void)
{
  for (int i = 0; i < 8; i++)
  {
      if (i==0)
      {
          dst_dict[i].year = 2024;
          dst_dict[i].dst_start = 1711846800; // (seconds since Jan 1, 1970)
          dst_dict[i].dst_end   = 1729994400;
      }
      else if (i == 1)
      {
          dst_dict[i].year = 2025;
          dst_dict[i].dst_start = 1743296400;
          dst_dict[i].dst_end   = 1761444000;
      }
      else if ( i == 2)
      {
          dst_dict[i].year = 2026;
          dst_dict[i].dst_start = 1774746000;
          dst_dict[i].dst_end   = 1792893600;
      }
      else if ( i == 3)
      {
          dst_dict[i].year = 2027;
          dst_dict[i].dst_start = 1806195600;
          dst_dict[i].dst_end   = 1824948000;
      }
      else if ( i == 4)
      {
          dst_dict[i].year = 2028;
          dst_dict[i].dst_start = 1837645200;
          dst_dict[i].dst_end   = 1856397600;
      }
      else if ( i == 5)
      {
          dst_dict[i].year = 2029;
          dst_dict[i].dst_start = 1869094800;
          dst_dict[i].dst_end   = 1887847200;
      }
      else if ( i == 6)
      {
          dst_dict[i].year = 2030;
          dst_dict[i].dst_start = 1901149200;
          dst_dict[i].dst_end   = 1919296800;
      }
      else if ( i == 6)
      {
          dst_dict[i].year = 2031;
          dst_dict[i].dst_start = 1932598800;
          dst_dict[i].dst_end   = 1950746400;
      }
  }
}

void disp_title(void)
{
  M5Cardputer.Display.setTextColor(YELLOW);
  M5Cardputer.Display.setTextSize(1);
  M5Cardputer.Display.setCursor(title_h, vert[0]-5);
  M5Cardputer.Display.print(TITLE);
  Serial.println(TITLE);
}

void disp_rtc_frame_page(void)
{
  M5Cardputer.Display.fillRect(hori[0], vert[1]-8, dw-5, dh-5, BLACK);
  M5Cardputer.Display.setCursor(hori[0], vert[1]);
  M5Cardputer.Display.print("RTC UTC: ");
  M5Cardputer.Display.setCursor(hori[0], vert[2]);
  M5Cardputer.Display.print("Weekday: ");
  M5Cardputer.Display.setCursor(hori[0], vert[3]);
  M5Cardputer.Display.print("Time: ");
}

void disp_esp32_frame_page(void)
{
  M5Cardputer.Display.fillRect(hori[0], vert[1]-8, dw-5, dh-5, BLACK);
  M5Cardputer.Display.setCursor(hori[0], vert[1]);
  M5Cardputer.Display.print("ESP32 LOC: ");
  M5Cardputer.Display.setCursor(hori[0], vert[2]);
  M5Cardputer.Display.print("Weekday: ");
  M5Cardputer.Display.setCursor(hori[0], vert[3]);
  M5Cardputer.Display.print("Time: ");
}

void disp_rtc_data_page(void)
{
  auto dt = M5.Rtc.getDateTime();

  M5Cardputer.Display.setCursor(hori[2], vert[1]);
  M5Cardputer.Display.printf("%04d-%02d-%02d"
    , dt.date.year
    , dt.date.month
    , dt.date.date);
    M5Cardputer.Display.setCursor(hori[2], vert[2]);
    M5Cardputer.Display.printf("%s", wd[dt.date.weekDay]);
    M5Cardputer.Display.fillRect(hori[2], vert[3]-8, dw-5, dh-5, BLACK);
    M5Cardputer.Display.setCursor(hori[2], vert[3]);
    M5Cardputer.Display.printf("%02d:%02d:%02d"
    , dt.time.hours
    , dt.time.minutes
    , dt.time.seconds);

  Serial.printf("RTC UTC:   %04d/%02d/%02d (%s)  %02d:%02d:%02d\r\n"
    , dt.date.year
    , dt.date.month
    , dt.date.date
    , wd[dt.date.weekDay]
    , dt.time.hours
    , dt.time.minutes
    , dt.time.seconds);
}

void disp_esp32_data_page(bool lStart)
{
  // ESP32 internal timer
  auto t = time(nullptr);
  auto tm = localtime(&t);

  if (!use_local_time)
    auto tm = gmtime(&t);  // default for UTC

  /*
  uint16_t yy = 1900;
  uint8_t  mo = 0;
  uint8_t  dd = 0;
  uint8_t  hh = 0;
  uint8_t  mi = 0;
  uint8_t  ss = 0;
  */

  if (lStart)
  {
     esp32_dt->yy = 1900;
     esp32_dt->mo = 0;
     esp32_dt->dd = 0;
     esp32_dt->wd = 0;
     esp32_dt->hh = 0;
     esp32_dt->mi = 0;
     esp32_dt->ss = 0;
  }

  String isdst = "\0";
  if (is_dst(tm->tm_year+1900))
    isdst = "dst";
  else
    isdst = "  ";

  if (esp32_dt->yy != tm->tm_year+1900 || esp32_dt->mo != tm->tm_mon || esp32_dt->dd != tm->tm_mday)
  {
    if (esp32_dt->yy != tm->tm_year+1900)
      esp32_dt->yy = tm->tm_year+1900;
    if (esp32_dt->mo != tm->tm_mon)
      esp32_dt->mo = tm->tm_mon;
    if (esp32_dt->dd != tm->tm_mday)
      esp32_dt->dd = tm->tm_mday;

    M5Cardputer.Display.fillRect(hori[2], vert[1]-8, dw-5, (vert[3]-8) - (vert[1]-8), BLACK);
    M5Cardputer.Display.setCursor(hori[2], vert[1]);
    M5Cardputer.Display.printf("%04d-%02d-%02d", tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday);
    M5Cardputer.Display.setCursor(hori[2], vert[2]);
    M5Cardputer.Display.printf("%s", wd[tm->tm_wday]);
  }
  if (esp32_dt->hh != tm->tm_hour || esp32_dt->mi != tm->tm_min || esp32_dt->ss != tm->tm_sec)
  {
    if (esp32_dt->hh != tm->tm_hour)
      esp32_dt->hh = tm->tm_hour;
    if (esp32_dt->mi != tm->tm_min)
      esp32_dt->mi = tm->tm_min;
    if (esp32_dt->ss != tm->tm_sec)
      esp32_dt->ss = tm->tm_sec;
    M5Cardputer.Display.fillRect(hori[2], vert[3]-8, dw-5, dh-5, BLACK);
    M5Cardputer.Display.setCursor(hori[2], vert[3]);
    M5Cardputer.Display.printf("%02d:%02d:%02d %s", tm->tm_hour, tm->tm_min, tm->tm_sec, isdst);
  }

  if (lStart)
    lStart = false;

  // For UTC time
  Serial.printf("ESP32 LOC: %04d/%02d/%02d (%s)  %02d:%02d:%02d %s\r\n",
    tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday,
    wd[tm->tm_wday],
    tm->tm_hour, tm->tm_min, tm->tm_sec, isdst);

  /* For local time
  auto tm = localtime(&t); // for local timezone.
  Serial.printf("ESP32 %s:%04d/%02d/%02d (%s)  %02d:%02d:%02d\r\n", NTP_TIMEZONE,
    tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday,
    wd[tm->tm_wday],
    tm->tm_hour, tm->tm_min, tm->tm_sec); 
  */
}

 void clr_disp_part(void) {
    M5Cardputer.Display.fillRect(0, vert[0] + 10, dw - 1, dh - 1, BLACK);  // clear display except title
  }

 void disp_msg(String msg, int linenr)
 {
    clr_disp_part();
    if (linenr == 1) 
    {
      M5Cardputer.Display.setTextSize(1);
      M5Cardputer.Display.setTextFont(&fonts::FreeSerif9pt7b);
      M5Cardputer.Display.setCursor(0, vert[2] - 10);
      M5Cardputer.Display.print(msg);
    }
    if (linenr == 2) 
    {
      M5Cardputer.Display.setTextSize(1);
      M5Cardputer.Display.setTextFont(&fonts::FreeSerif9pt7b);
      M5Cardputer.Display.setCursor(0, vert[3] - 10);
      M5Cardputer.Display.print(msg);
    }
    Serial.println(msg);
    delay(2000);
  }

bool is_dst(int yr)
{
  bool ret = false;
  bool yr_fnd = false;
  time_t currentTime = time(NULL);
  time_t dst_start1 = 0;
  time_t dst_end1 = 0;
  int le = sizeof(dst_dict)/sizeof(dst_dict[0]);
  Serial.print("is_dst(): size of dst_dict = ");
  Serial.println(le);

  Serial.print("yr = ");
  Serial.println(yr);
  for (int i = 0; i < le; i++)
  {
    Serial.print("dst_dict[");
    Serial.print(i);
    Serial.print("].year = ");
    Serial.println(dst_dict[i].year);
    if (yr == dst_dict[i].year)
    {
      dst_start1 = dst_dict[i].dst_start;
      dst_end1 = dst_dict[i].dst_end;
      yr_fnd = true;
      break;
    }
  }

  if (yr_fnd)
  {
    Serial.println("is_dst():");
    Serial.print("dst_start1  = ");
    Serial.println(dst_start1);
    Serial.print("currentTime = ");
    Serial.println(currentTime);
    Serial.print("dst_end1    = ");
    Serial.println(dst_end1);
    if (currentTime > dst_start1 && currentTime < dst_end1)
      ret = true;
    Serial.print("is_dst(): return value = ");
    Serial.println(ret);
    Serial.println();
  }
  return ret;
}

void setup(void)
{
  auto cfg = M5.config();

  cfg.output_power = true; 
  cfg.external_rtc  = true;  // default=false.

  M5Cardputer.begin(cfg, true);
  M5Cardputer.Power.begin();
  dw = M5Cardputer.Display.width();
  dh = M5Cardputer.Display.height();

  Serial.begin(115200);

  load_dst_dict();  // fill the dst dictionary

  int le = sizeof(TITLE)/sizeof(TITLE[0]);
  title_h = (dh - le) / 4;

  M5Cardputer.Display.setRotation(1);
  M5Cardputer.Display.setTextFont(&fonts::FreeSerif9pt7b);
  M5Cardputer.Display.setTextColor(GREEN);
  M5Cardputer.Display.clear();
  disp_title();

  M5Cardputer.Display.setEpdMode(m5gfx::epd_fastest);

  if (!M5.Rtc.isEnabled())
  {
    M5Cardputer.Display.setTextColor(RED);
    M5Cardputer.Display.setCursor(hori[0], vert[1]);
    char rtcNFnd[] = "External RTC not found";
    M5Cardputer.Display.println(rtcNFnd);
    Serial.println(rtcNFnd);
    for (;;) { M5.delay(500); }  // Loop forever
  }
  else
  {
    M5Cardputer.Display.setTextColor(GREEN);
    M5Cardputer.Display.setCursor(hori[0], vert[1]);
    char rtcFnd[] = "External RTC found";
    M5Cardputer.Display.println(rtcFnd);
    Serial.println(rtcFnd);
  }

// It is recommended to set UTC for the RTC and ESP32 internal clocks.

/* // setup RTC ( direct setting )
  //                      YYYY  MM  DD      hh  mm  ss
  M5.Rtc.setDateTime( { { 2021, 12, 31 }, { 12, 34, 56 } } );
*/

 // setup RTC ( NTP auto setting )
 // From Microsoft CoPilot: 
 // const char* ntpServer = "pool.ntp.org";
 const long gmtOffset_sec = 0;
 int daylightOffset_sec = 0;  // default GMT 
 if (use_local_time)
  daylightOffset_sec = 3600;  // set for local

 configTime(gmtOffset_sec, daylightOffset_sec, NTP_SERVER1); // ,ntpServer);
 // end from Microsoft CoPilot
 // configTzTime(NTP_TIMEZONE, NTP_SERVER1, NTP_SERVER2, NTP_SERVER3);

#ifdef WiFi_h
  M5Cardputer.Display.setCursor(hori[0], vert[4]);
  M5Cardputer.Display.print("WiFi:");
  Serial.println("Connecting WiFi");
  //Serial.println(WIFI_SSID);
  WiFi.begin( WIFI_SSID, WIFI_PASSWORD );

  for (int i = 20; i && WiFi.status() != WL_CONNECTED; --i)
  {
    M5Cardputer.Display.print(".");
    Serial.print(".");
    delay(500);
  }
  if (WiFi.status() == WL_CONNECTED) {
    M5Cardputer.Display.fillRect(hori[0], vert[1]-8, dw-5, dh-5, BLACK);
    M5Cardputer.Display.setCursor(hori[0], vert[1]);
    char wct[] = "WiFi Connected to: ";
    M5Cardputer.Display.println(wct);
    Serial.println();
    Serial.print(wct);
    Serial.println(WIFI_SSID);
    IPAddress ip;
    ip = WiFi.localIP();
    Serial.print("IP address: ");
    Serial.println(ip);
    Serial.println();
    M5Cardputer.Display.setCursor(hori[0], vert[2]);
    M5Cardputer.Display.println(WIFI_SSID);
    M5Cardputer.Display.setCursor(hori[0], vert[3]);
    M5Cardputer.Display.print("IP: ");
    M5Cardputer.Display.print(ip);
    M5Cardputer.Display.setCursor(hori[0], vert[4]);
    Serial.println("Connecting NTP");
    M5Cardputer.Display.print("NTP:");
#if SNTP_ENABLED
    while (sntp_get_sync_status() != SNTP_SYNC_STATUS_COMPLETED)
    {
      M5Cardputer.Display.print(".");
      Serial.print(".");
      delay(1000);
    }
#else
    delay(1600);
    struct tm timeInfo;
    while (!getLocalTime(&timeInfo, 1000))
    {
      M5Cardputer.Display.print('.');
      Serial.print(".");
    };
#endif
    M5Cardputer.Display.fillRect(hori[0], vert[3]-8, dw-5, dh-5, BLACK);
    M5Cardputer.Display.setCursor(hori[0], vert[3]);
    char ntpc[] = "NTP Connected.";
    M5Cardputer.Display.println(ntpc);
    Serial.println();
    Serial.println(ntpc);

    time_t t = time(nullptr)+1; // Advance one second.
    while (t > time(nullptr));  /// Synchronization in seconds
    M5.Rtc.setDateTime( gmtime( &t ) );
  }
  else
  {
    M5Cardputer.Display.fillRect(hori[0], vert[1]-8, dw-5, dh-5, BLACK);
    M5Cardputer.Display.setCursor(hori[0], vert[1]);
    M5Cardputer.Display.println("WiFi none...");
  }
#endif

  M5Cardputer.Display.fillRect(hori[0], vert[1]-8, dw-5, dh-5, BLACK);
}

void loop(void)
{
  int page_nr = 1;
  char kp[] = "key pressed";
  char kpp[] = "key p pressed"; 
  bool lStart = true;
  bool lPage1_shown = false;
  bool lPage2_shown = false;

  while (true)
  {
    M5Cardputer.update();
    if (M5Cardputer.Keyboard.isChange()) {
      // KEY_FN
      if ( M5Cardputer.Keyboard.isKeyPressed('p') || M5Cardputer.Keyboard.isKeyPressed('P') )
      {
        page_nr++;
        lStart = true;
        if (page_nr > 2)
          page_nr = 1;
        if (page_nr == 1)
          lPage1_shown = false;
        if (page_nr == 2)
          lPage2_shown = false;
        disp_msg(kpp, 2);
        Serial.println(kpp);
      }
      else if (M5Cardputer.Keyboard.isPressed()) 
      {
        disp_msg(kp, 2);
        Serial.println(kp);
      }
    }

    if (page_nr == 1)
    {
      if (!lPage1_shown)
      {
        lPage1_shown = true;
        disp_esp32_frame_page();
      }
      disp_esp32_data_page(lStart);
      lStart = false;
    }
    else if (page_nr == 2)
    {
      if (!lPage2_shown)
      {
        lPage2_shown = true;
        disp_rtc_frame_page();
      }
      disp_rtc_data_page();
    }
    delay(500);
  }
}

#if !defined ( ARDUINO )
extern "C" {
  void loopTask(void*)
  {
    setup();
    for (;;) {
      loop();
    }
    vTaskDelete(NULL);
  }

  void app_main()
  {
    xTaskCreatePinnedToCore(loopTask, "loopTask", 8192, NULL, 1, NULL, 1);
  }
}
#endif
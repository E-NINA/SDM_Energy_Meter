//dds238 live page example by enina

#define READdds238EVERY  2000                                                      //read dds238 every 2000ms
#define NBREG   5                                                               //number of sdm registers to read
//#define USE_STATIC_IP

/*  WEMOS D1 Mini
                     ______________________________
                    |   L T L T L T L T L T L T    |
                    |                              |
                 RST|                             1|TX HSer
                  A0|                             3|RX HSer
                  D0|16                           5|D1
                  D5|14                           4|D2
                  D6|12                    10kPUP_0|D3
RX SSer/HSer swap D7|13                LED_10kPUP_2|D4
TX SSer/HSer swap D8|15                            |GND
                 3V3|__                            |5V
                       |                           |
                       |___________________________|
*/

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>

#include <ESPAsyncTCP.h>                                                        //https://github.com/me-no-dev/ESPAsyncTCP
#include <ESPAsyncWebServer.h>                                                  //https://github.com/me-no-dev/ESPAsyncWebServer

#include <SoftwareSerial.h>                                                     //import SoftwareSerial library (if used)
#include <dds238.h>                                                                //https://github.com/E-NINA/dds238_Energy_Meter

#include "index_page.h"
//------------------------------------------------------------------------------
AsyncWebServer server(80);

SoftwareSerial swSerdds238(13, 12);                                                //config SoftwareSerial (rx->pin13 / tx->pin15) (if used)

dds238 dds238(swSerdds238, 9600, NOT_A_PIN);                                             //SOFTWARE SERIAL
//dds238 dds238(Serial, 9600, NOT_A_PIN, SERIAL_8N1, false);                            //HARDWARE SERIAL

//------------------------------------------------------------------------------
String devicename = "PWRMETER";

#if defined ( USE_STATIC_IP )
IPAddress ip(192, 168, 0, 130);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
#endif

const char* wifi_ssid = "YOUR_SSID";
const char* wifi_password = "YOUR_PASSWORD";

String lastresetreason = "";

unsigned long readtime;
//------------------------------------------------------------------------------
typedef volatile struct {
  volatile float regvalarr;
  const uint16_t regarr;
  byte isSigned;
} dds238_struct;

volatile dds238_struct dds238arr[NBREG] = {
  {0.00, dds238_VOLTAGE, 2},                                                      //V
  {0.00, dds238_CURRENT, 2},                                                      //A
  {0.00, dds238_POWER, 2},                                                        //W
  {0.00, dds238_POWER_FACTOR, 2},                                                 //PF
  {0.00, dds238_FREQUENCY, 2},                                                    //Hz
};
//------------------------------------------------------------------------------
void xmlrequest(AsyncWebServerRequest *request) {
  String XML = F("<?xml version='1.0'?><xml>");
  for (int i = 0; i < NBREG; i++) { 
    XML += "<response" + (String)i + ">";
    XML += String(dds238arr[i].regvalarr,2);
    XML += "</response" + (String)i + ">";
  }
  XML += F("<freeh>");
  XML += String(ESP.getFreeHeap());
  XML += F("</freeh>");
  XML += F("<rst>");
  XML += lastresetreason;
  XML += F("</rst>");
  XML += F("</xml>");
  request->send(200, "text/xml", XML);
}
//------------------------------------------------------------------------------
void indexrequest(AsyncWebServerRequest *request) {
  request->send_P(200, "text/html", index_page); 
}
//------------------------------------------------------------------------------
void ledOn() {
  digitalWrite(LED_BUILTIN, LOW);
}
//------------------------------------------------------------------------------
void ledOff() {
  digitalWrite(LED_BUILTIN, HIGH);
}
//------------------------------------------------------------------------------
void ledSwap() {
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}
//------------------------------------------------------------------------------
void otaInit() {
  ArduinoOTA.setHostname(devicename.c_str());

  ArduinoOTA.onStart([]() {
    ledOn();
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    ledSwap();
  });
  ArduinoOTA.onEnd([]() {
    ledOff();
  });
  ArduinoOTA.onError([](ota_error_t error) {
    ledOff();
  });
  ArduinoOTA.begin();
}
//------------------------------------------------------------------------------
void serverInit() {
  server.on("/", HTTP_GET, indexrequest);
  server.on("/xml", HTTP_PUT, xmlrequest);
  server.onNotFound([](AsyncWebServerRequest *request){
    request->send(404);
  });
  server.begin();
}
//------------------------------------------------------------------------------
static void wifiInit() {
  WiFi.persistent(false);                                                       // Do not write new connections to FLASH
  WiFi.mode(WIFI_STA);
#if defined ( USE_STATIC_IP )
  WiFi.config(ip, gateway, subnet);                                             // Set fixed IP Address
#endif
  WiFi.begin(wifi_ssid, wifi_password);

  while( WiFi.status() != WL_CONNECTED ) {                                      //  Wait for WiFi connection
    ledSwap();
    delay(100);
  }
}
//------------------------------------------------------------------------------
void dds238Read() {
  float tmpval = NAN;

  for (uint8_t i = 0; i < NBREG; i++) {
    tmpval = dds238.readVal(dds238arr[i].regarr, dds238arr[i].isSigned);

    if (isnan(tmpval))
      dds238arr[i].regvalarr = 0.00;
    else
      dds238arr[i].regvalarr = tmpval;
    yield();
  }
}
//------------------------------------------------------------------------------
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  ledOn();

  lastresetreason = ESP.getResetReason();

  wifiInit();
  otaInit();
  serverInit();
  dds238.begin();

  readtime = millis();

  ledOff();
}
//------------------------------------------------------------------------------
void loop() {
  ArduinoOTA.handle();

  if (millis() - readtime >= READdds238EVERY) {
    dds238Read();
    readtime = millis();
  }

  yield();
}

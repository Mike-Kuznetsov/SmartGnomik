/* Mautoz Tech Smart AC
 * Enter your WiFi SSID and Password below, also don't forget to change rawData and state commands (for ON and OFF commands separately).
 * To get commands for your AC use another program in this repository.
 * Mautoz Tech https://www.youtube.com/c/MautozTech
 * Заметки ESPшника (Video about this program is there) - https://www.youtube.com/@ESPdev
 * 
 * Used components: 
 * ESP8266 (Not tested with ESP32)
 * IR Transmitter HX-53
 * Smartphone Powersupply 
 * 
 * Licensed under GNU GPL V3: This program is a free software, you are free to use or modify the code. No warranty.
 */
const char* ssid = "Wi-Fi Name";
const char* password = "PASSWORD";
 
#ifdef ESP32
  #include <WiFi.h>
  #include <WiFiClient.h>
  #include <WebServer.h>
  WebServer server(80);
#else
  #include <ESP8266WiFi.h>
  #include <WiFiClient.h>
  #include <ESP8266WebServer.h>
  ESP8266WebServer server(80);
#endif
#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
const uint16_t kIrLed = 4;  // ESP8266 GPIO pin to use. Recommended: 4 (D2).
IRsend irsend(kIrLed);  // Set the GPIO to be used to sending the message.
uint16_t rawDataON[243] = {8336, 4212, 516, 548, 516, 1614, 514, 1614, 516, 548, 516, 1614, 516, 548, 516, 1614, 514, 550, 516, 550, 514, 550, 514, 550, 514, 550, 514, 1616, 514, 1614, 516, 1614, 516, 548, 516, 548, 516, 548, 514, 550, 516, 548, 516, 550, 514, 550, 516, 548, 514, 550, 516, 550, 514, 548, 516, 550, 514, 550, 516, 548, 516, 548, 516, 550, 516, 548, 514, 550, 516, 1614, 516, 550, 514, 548, 516, 550, 516, 1614, 516, 548, 514, 550, 516, 1614, 516, 1614, 516, 548, 516, 550, 514, 550, 514, 550, 514, 550, 514, 550, 514, 550, 516, 548, 516, 548, 516, 550, 514, 550, 514, 550, 516, 548, 514, 550, 516, 548, 516, 548, 516, 550, 514, 550, 514, 550, 514, 550, 514, 550, 516, 548, 516, 548, 516, 548, 516, 548, 516, 550, 514, 550, 516, 550, 514, 548, 516, 548, 516, 550, 514, 548, 516, 550, 514, 550, 514, 548, 514, 550, 516, 548, 516, 550, 516, 548, 514, 550, 514, 550, 514, 548, 516, 548, 516, 548, 516, 548, 516, 550, 514, 550, 514, 550, 514, 548, 516, 550, 514, 548, 516, 548, 516, 548, 514, 550, 516, 548, 516, 548, 514, 550, 514, 550, 514, 550, 516, 550, 514, 550, 514, 550, 514, 548, 516, 548, 516, 550, 514, 548, 516, 548, 516, 548, 516, 550, 514, 548, 516, 1614, 516, 548, 516, 548, 516, 1614, 514, 1614, 514, 548, 514, 550, 516, 550, 514}; // MIRAGE
uint8_t stateON[15] = {0x56, 0x70, 0x00, 0x00, 0x22, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x19};
uint16_t rawDataOFF[243] = {8334, 4210, 518, 548, 516, 1614, 516, 1612, 516, 546, 518, 1614, 516, 548, 516, 1614, 516, 548, 516, 548, 516, 548, 516, 548, 516, 548, 516, 1612, 516, 1614, 516, 1612, 518, 548, 516, 548, 516, 548, 516, 550, 516, 548, 516, 548, 516, 548, 516, 548, 516, 548, 516, 548, 516, 548, 516, 548, 516, 548, 518, 548, 516, 548, 518, 548, 516, 548, 516, 548, 516, 1614, 516, 548, 516, 548, 516, 548, 516, 1614, 516, 546, 516, 548, 516, 1612, 518, 1614, 518, 546, 518, 548, 518, 546, 516, 548, 518, 1612, 516, 1612, 516, 548, 518, 548, 516, 546, 518, 546, 516, 548, 516, 548, 516, 548, 516, 548, 516, 548, 516, 546, 518, 548, 516, 548, 516, 548, 516, 548, 516, 548, 516, 548, 516, 548, 516, 548, 516, 546, 518, 548, 516, 548, 516, 548, 514, 548, 516, 548, 516, 548, 516, 548, 516, 548, 516, 548, 516, 548, 516, 548, 516, 548, 516, 548, 516, 548, 516, 548, 516, 548, 516, 548, 516, 548, 516, 548, 516, 548, 516, 548, 516, 546, 518, 548, 516, 548, 516, 548, 518, 546, 516, 548, 516, 548, 516, 548, 518, 546, 518, 548, 516, 548, 518, 548, 516, 548, 516, 548, 518, 546, 516, 548, 516, 548, 516, 548, 516, 548, 516, 548, 516, 548, 516, 548, 516, 548, 516, 548, 518, 1612, 516, 548, 516, 1614, 516, 548, 516, 548, 516, 1614, 516, 548, 516, 548, 516}; // MIRAGE
uint8_t stateOFF[15] = {0x56, 0x70, 0x00, 0x00, 0x22, 0xC3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x25};


void setup(void){
  //pinMode(LED_BUILTIN, OUTPUT);
  pinMode(0, OUTPUT);
  digitalWrite(0, HIGH);
  WiFi.begin(ssid, password);
  irsend.begin();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  server.on("/", [](){
    server.send(200, "text/html", "<head><title>AC</title></head><p>Mautoz Tech Smart AC.<br><a href=\"turn_on\"><button>ON</button></a>&nbsp;<a href=\"turn_off\"><button>OFF</button></a></p>");
  });
  server.on("/turn_on", [](){
    server.send(200, "text/html", "<head><title>AC</title></head><p>Mautoz Tech Smart AC.<br><a href=\"turn_on\"><button>ON</button></a>&nbsp;<a href=\"turn_off\"><button>OFF</button></a><br>AC has been turned on</p>");
    //digitalWrite(LED_BUILTIN, HIGH);
    irsend.sendRaw(rawDataON, 243, 38);  // Send a raw data capture at 38kHz.
    irsend.sendMirage(stateON);
  });
  server.on("/turn_off", [](){
    server.send(200, "text/html", "<head><title>AC</title></head><p>Mautoz Tech Smart AC.<br><a href=\"turn_on\"><button>ON</button></a>&nbsp;<a href=\"turn_off\"><button>OFF</button></a><br>AC has been turned off</p>");
    irsend.sendRaw(rawDataOFF, 243, 38);  // Send a raw data capture at 38kHz.
    irsend.sendMirage(stateOFF);
  });
  server.begin();
}
 
void loop(void){
  server.handleClient();
}

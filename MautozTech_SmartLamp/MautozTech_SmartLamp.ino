/* Mautoz Tech Smart Lamp
 *
 * Mautoz Tech https://www.youtube.com/c/MautozTech
 * Заметки ESPшника (Video about this program is there) - https://www.youtube.com/@ESPdev
 * 
 * Used components: 
 * ESP8266
 * Relay 3.3V
 * Smartphone Powersupply 
 * 
 * Licensed under GNU GPL V3: This program is a free software, you are free to use or modify the code. No warranty.
 */
const char* ssid = "Wi-Fi";
const char* password = "PASSWORD";
int lampID = 1; 
 
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

void setup(void){
  //pinMode(LED_BUILTIN, OUTPUT);
  pinMode(0, OUTPUT);
  digitalWrite(0, HIGH);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  server.on("/", [](){
    server.send(200, "text/html", "<head><title>Lamp "+String(lampID)+"</title></head><p>Mautoz Tech Smart Light. ID: "+String(lampID)+"<br><a href=\"turn_on\"><button>ON</button></a>&nbsp;<a href=\"turn_off\"><button>OFF</button></a></p>");
  });
  server.on("/turn_on", [](){
    server.send(200, "text/html", "<head><title>Lamp "+String(lampID)+"</title></head><p>Mautoz Tech Smart Light. ID: "+String(lampID)+"<br><a href=\"turn_on\"><button>ON</button></a>&nbsp;<a href=\"turn_off\"><button>OFF</button></a><br>Lamp has been turned on</p>");
    //digitalWrite(LED_BUILTIN, HIGH);
    digitalWrite(0, LOW);
    delay(200);
    digitalWrite(0, HIGH);
    delay(200);
  });
  server.on("/turn_off", [](){
    server.send(200, "text/html", "<head><title>Lamp "+String(lampID)+"</title></head><p>Mautoz Tech Smart Light. ID: "+String(lampID)+"<br> <a href=\"turn_on\"><button>ON</button></a>&nbsp;<a href=\"turn_off\"><button>OFF</button></a><br>Lamp has been turned off</p>");
    //digitalWrite(LED_BUILTIN, LOW);
    digitalWrite(0, HIGH);
    delay(200);
    digitalWrite(0, LOW);
    delay(200);
  });
  server.begin();
}
 
void loop(void){
  server.handleClient();
}

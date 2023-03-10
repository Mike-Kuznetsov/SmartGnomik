/* Program 4. For BMP280 sensor with web page, night mode and API for my 'Kamilya Gnomik' Orange Pi voice recongizer.
 * Central water heating system controller for your room.
 *
 * Mautoz Tech https://www.youtube.com/channel/UCWN_KT_CAjGZQG5grHldL8w
 * Заметки ESPшника (Video about this program is there) - https://www.youtube.com/channel/UCQAbEIaWFdARXKqcufV6y_g
 * 
 * Functions: 
 * handleRoot() - starts when somebody requests ESP webpage from web browser and processes request
 * handleSave() - starts when somebody presses "save" on the ESP webpage and saves new settings into permanent memory
 * handleChange() - Saves new temperature after request from Orange Pi has been received
 * handleTemp() - sends current parameters to a web browser
 * eepromRead() - reads settings from permanent memory after ESP starts
 * wait(long waitingTime) - it's like delay() function but it doesnt stop webserver
 * motorWait(long waitingTime) - the same but without opportunity to break it via cycleBreak variable
 * printInfo() - sends information via Serial port (speed 9600) between wait function requests.
 * isNight() - if it's night returns True if it's day returns False
 * sendNTPpacket() - asks NTP (time) server to get current time
 * reverse() / forward() - rotate the motor
 * 
 * Used components: 
 * Microcontroller: ESP8266 NodeMCU,
 * Motor: 28BUJ-48, 
 * Motor controller shield: L298N, 
 * Temperature Sensor: DHT11
 * 
 * Licensed under GNU GPL V3: This program is a free software, you are free to use or modify the code. No warranty.
 */
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <ESP_EEPROM.h>
#include <WiFiUdp.h>
Adafruit_BMP280 bmp; 
const int ENA = 15;
const int IN1 = 13;
const int IN2 = 12;
const int IN4 = 14;
const int IN3 = 0;
const int ENB = 16;

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
ESP8266WebServer server(80);


struct Settings{
  int pos;
  
  float targetTemp;
  float deviation;
  long delayTime;
  
  float nightTargetTemp;
  float nightDeviation;
  long nightDelayTime;
  
  int nightStartHour;
  int nightStartMinute;
  int nightEndHour;
  int nightEndMinute;
} eeprom;

int outputDelay = 5000;
bool cycleBreak = false;
char htmlResponse[3000];
char htmlResponse2[100];
char htmlResponseTemp[100];
float temp;
const char* ssid = "Wi-Fi";
const char* password = "PASSWORD";

unsigned int localPort = 2390; 
IPAddress timeServerIP; 
const char* ntpServerName = "time.windows.com";
const int NTP_PACKET_SIZE = 48;
byte packetBuffer[ NTP_PACKET_SIZE];
WiFiUDP udp;

void handleRoot() {
  //Serial.println("HANDLING ROOT STARTED");
  snprintf ( htmlResponse, 5000,
  "<!DOCTYPE html>\
  <html lang=\"en\">\
    <head>\
      <meta charset=\"utf-8\">\
      <title>Heater Manager</title>\
      <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\
    </head>\
    <body>\
            <h1>House Heater Manager</h1>\
            <div id=\'receivedTempFromServer\'>\
            </div>\
            <br>Target Temperature: <input type='text' id='setTargetTemp' value=           size=3 autofocus>\
            Deviation: <input type='text' id='setDeviation' value=           size=3 autofocus>\
            Delay: <input type='text' value=           id='setDelayTime' size=4 autofocus>\
            <br>\
            <br>Night mode starts at: <input type='text' id='setNightStartHour' value=           size=3 autofocus>\
            <input type='text' id='setNightStartMinute' value=           size=3 autofocus>\
            Ends at: <input type='text' value=           id='setNightEndHour' size=3 autofocus>\
            <input type='text' value=           id='setNightEndMinute' size=3 autofocus>\
            <br>\
            <br>Night Temperature: <input type='text' id='setNightTargetTemp' value=           size=3 autofocus>\
            Deviation: <input type='text' id='setNightDeviation' value=           size=3 autofocus>\
            Delay: <input type='text' value=           id='setNightDelayTime' size=4 autofocus>\
            <div>\<br><button id=\"send_button\">Save</button>\
            </div>\
            <div id=\'receivedMessageFromServer\'>\
            </div>\
      <script src=\"https://ajax.googleapis.com/ajax/libs/jquery/1.11.3/jquery.min.js\"></script>\    
      <script>\
        $('#send_button').click(function(e){\
          var targetTemp = $('#setTargetTemp').val();\
          var deviation = $('#setDeviation').val();\
          var delayTime = $('#setDelayTime').val();\    
          var nightTargetTemp = $('#setNightTargetTemp').val();\
          var nightDeviation = $('#setNightDeviation').val();\
          var nightDelayTime = $('#setNightDelayTime').val();\
          var nightStartHour = $('#setNightStartHour').val();\
          var nightStartMinute = $('#setNightStartMinute').val();\
          var nightEndHour = $('#setNightEndHour').val();\
          var nightEndMinute = $('#setNightEndMinute').val();\
          e.preventDefault();\     
          $.get('/save?targetTemp=' + targetTemp + '&deviation=' + deviation + '&delayTime=' + delayTime + '&nightTargetTemp=' + nightTargetTemp + '&nightDeviation=' + nightDeviation + '&nightDelayTime=' + nightDelayTime + '&nightStartHour=' + nightStartHour + '&nightStartMinute=' + nightStartMinute + '&nightEndHour=' + nightEndHour + '&nightEndMinute=' + nightEndMinute, function(data){\
          $('#receivedMessageFromServer').html(data);\
          });\
        });\  
        $.get('/getTemp', function(data){\
            $('#receivedTempFromServer').html(data);\
            });\
        setInterval(function(){\ 
          $.get('/getTemp', function(data){\
            $('#receivedTempFromServer').html(data);\
            });\
          }, 5000);\ 
      </script>\
    </body>\
  </html>"); 
  Serial.println("HANDLING ROOT");
  String myTempStr = "'"+String(eeprom.targetTemp)+"'";
  String deviationStr = "'"+String(eeprom.deviation)+"'";
  String delayTimeStr = "'"+String(eeprom.delayTime)+"'";
  String nightTempStr = "'"+String(eeprom.nightTargetTemp)+"'";
  String nightDeviationStr = "'"+String(eeprom.nightDeviation)+"'";
  String nightDelayTimeStr = "'"+String(eeprom.nightDelayTime)+"'";

  String nightStartHourStr = "'"+String(eeprom.nightStartHour)+"'";
  String nightStartMinuteStr = "'"+String(eeprom.nightStartMinute)+"'";
  String nightEndHourStr = "'"+String(eeprom.nightEndHour)+"'";
  String nightEndMinuteStr = "'"+String(eeprom.nightEndMinute)+"'";
  for (int i=0; i<myTempStr.length(); i++){
    htmlResponse[385+i] = myTempStr[i];
  }
  for (int i=0; i<deviationStr.length(); i++){
    htmlResponse[479+i] = deviationStr[i];
  }
  for (int i=0; i<delayTimeStr.length(); i++){
    htmlResponse[553+i] = delayTimeStr[i];
  }
  for (int i=0; i<nightStartHourStr.length(); i++){
    htmlResponse[700+i] = nightStartHourStr[i];
  }
  for (int i=0; i<nightStartMinuteStr.length(); i++){
    htmlResponse[790+i] = nightStartMinuteStr[i];
  }
  for (int i=0; i<nightEndHourStr.length(); i++){
    htmlResponse[870+i] = nightEndHourStr[i]; //53
  }
  for (int i=0; i<nightEndMinuteStr.length(); i++){
    htmlResponse[950+i] = nightEndMinuteStr[i];
  }
  for (int i=0; i<nightTempStr.length(); i++){
    htmlResponse[1103+i] = nightTempStr[i];
  }
  for (int i=0; i<nightDeviationStr.length(); i++){
    htmlResponse[1199+i] = nightDeviationStr[i];
  }
  for (int i=0; i<nightDelayTimeStr.length(); i++){
    htmlResponse[1271+i] = nightDelayTimeStr[i];
  }
  //Serial.println(htmlResponse);
  server.send (200, "text/html", htmlResponse);  
  //Serial.println("HANDLING ENDED");
  //server.send (200, "text/html", "KEK");  
}

void handleSave() {
  if (server.args()==10){
    if (server.arg("deviation")!=""){
      float receivedDeviation = server.arg("deviation").toFloat();
      if (receivedDeviation>0.1 && receivedDeviation<2){
        eeprom.deviation = receivedDeviation;
      }  
    }
    if (server.arg("delayTime")!=""){
      long receivedDelayTime = server.arg("delayTime").toInt();
      if (receivedDelayTime>1000 && receivedDelayTime<36000000){
        eeprom.delayTime = receivedDelayTime;
      }  
    }
    if (server.arg("targetTemp")!=""){
      float receivedtargetTemp = server.arg("targetTemp").toFloat();
      if (receivedtargetTemp>20 && receivedtargetTemp<30){
        eeprom.targetTemp = receivedtargetTemp;
      }  
    }
    if (server.arg("nightDeviation")!=""){
      float receivedNightDeviation = server.arg("nightDeviation").toFloat();
      if (receivedNightDeviation>0.1 && receivedNightDeviation<2){
        eeprom.nightDeviation = receivedNightDeviation;
      }  
    }
    if (server.arg("nightDelayTime")!=""){
      long receivedNightDelayTime = server.arg("nightDelayTime").toInt();
      if (receivedNightDelayTime>1000 && receivedNightDelayTime<36000000){
        eeprom.nightDelayTime = receivedNightDelayTime;
      }  
    }
    if (server.arg("nightTargetTemp")!=""){
      float receivedNightTargetTemp = server.arg("nightTargetTemp").toFloat();
      if (receivedNightTargetTemp>20 && receivedNightTargetTemp<30){
        eeprom.nightTargetTemp = receivedNightTargetTemp;
      }  
    }
    if (server.arg("nightStartHour")!=""){
      float receivedNightStartHour = server.arg("nightStartHour").toFloat();
      if (receivedNightStartHour>20 && receivedNightStartHour<30){
        eeprom.nightStartHour = receivedNightStartHour;
      }  
    }
    if (server.arg("nightStartMinute")!=""){
      float receivedNightStartMinute = server.arg("nightStartMinute").toFloat();
      if (receivedNightStartMinute>0.1 && receivedNightStartMinute<2){
        eeprom.nightStartMinute = receivedNightStartMinute;
      }  
    }
    if (server.arg("nightEndHour")!=""){
      long receivedNightEndHour = server.arg("nightEndHour").toInt();
      if (receivedNightEndHour>1000 && receivedNightEndHour<36000000){
        eeprom.nightEndHour = receivedNightEndHour;
      }  
    }
    if (server.arg("nightEndMinute")!=""){
      float receivedNightEndMinute = server.arg("nightEndMinute").toFloat();
      if (receivedNightEndMinute>20 && receivedNightEndMinute<30){
        eeprom.nightEndMinute = receivedNightEndMinute;
      }  
    }
    EEPROM.put(0, eeprom);
    EEPROM.commit();
    cycleBreak = true;
    snprintf ( htmlResponse2, 3000, "Your settings have been saved successfully");
    server.send (200, "text/html", htmlResponse2);  
  }  
}

void handleChange() {
  if (server.args()==1){
    if (server.arg("temp")!=""){
      float changeTemp = server.arg("temp").toFloat();
      if (changeTemp>=-5 && changeTemp<=5){
        eeprom.targetTemp = eeprom.targetTemp+changeTemp;
      }  
    }
    
    EEPROM.put(0, eeprom);
    EEPROM.commit();
    cycleBreak = true;
    snprintf ( htmlResponse2, 3000, "Your settings have been saved successfully");
    server.send (200, "text/html", htmlResponse2);  
  }  
}

void handleTemp() {
  //snprintf ( htmlResponseTemp, 3000, "Temperature:"+String(temp)+" Position:"+String(eeprom.pos)+" TargetTemperature:"+String(eeprom.targetTemp)+" Deviation:"+String(eeprom.deviation)+" DelayTime:"+String(eeprom.delayTime));
  Serial.println("Handling Temperature");
  server.send (200, "text/html", "Temperature:"+String(temp)+" Position:"+String(eeprom.pos)); 
}

void eepromRead(){
  if(EEPROM.percentUsed()>=0){
    EEPROM.get(0, eeprom);
    Serial.println("Reading settings from EEPROM");
    Serial.println("Pos:");
    Serial.println(eeprom.pos);
    Serial.println("targetTemp:");
    Serial.println(eeprom.targetTemp);
    Serial.println("deviation:");
    Serial.println(eeprom.deviation);
    Serial.println("delayTime:");
    Serial.println(eeprom.delayTime);
  } 
  else{
    eeprom.pos = 2;
    eeprom.targetTemp = 24.5;
    eeprom.deviation = 0.2;
    eeprom.delayTime = 1800000;
    eeprom.nightTargetTemp = 24;
    eeprom.nightDeviation = 0.5;
    eeprom.nightDelayTime = 3600000;
    eeprom.nightStartHour = 22;
    eeprom.nightStartMinute = 30;
    eeprom.nightEndHour = 8;
    eeprom.nightEndMinute = 0;
    EEPROM.put(0, eeprom);
    EEPROM.commit();
    Serial.println("Applying default settings");
  }
}

void wait(long waitingTime){
  long waitingStartTime = millis();
  while (millis() - waitingStartTime < waitingTime){
    server.handleClient();
    if (cycleBreak == true){
      break;
    }
    //delay(0);
  }
}

void motorWait(long waitingTime){
  long waitingStartTime = millis();
  while (millis() - waitingStartTime < waitingTime){
    server.handleClient();
    //delay(0);
  }
}

void printInfo(long delayTim){
  float cycleTimes = delayTim / outputDelay;
  for (int t = 0; t < cycleTimes; t++){
    temp = bmp.readTemperature();
    Serial.print("Temperature:");
    Serial.print(temp);
    Serial.print(" Position:");
    Serial.print(eeprom.pos);
    Serial.print(" TargetTemperature:");
    Serial.print(eeprom.targetTemp);
    Serial.print(" Deviation:");
    Serial.print(eeprom.deviation);
    Serial.print(" DelayTime:");
    Serial.println(eeprom.delayTime);
    wait(outputDelay);
    if (cycleBreak == true){
      cycleBreak = false;
      break;
    }
  }
}

void setup()
{
  Serial.begin(9600);
  bool status = bmp.begin(0x76);  
  if (!status) {
    Serial.println("Could not find a valid BMp280 sensor, check wiring!");
    while (1);
  }
  EEPROM.begin(sizeof(Settings));
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  eepromRead();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  server.on ( "/", handleRoot);
  server.on ("/save", handleSave);
  server.on ("/getTemp", handleTemp);
  server.on ("/changeTemp", handleChange);
  server.begin();
  Serial.println("HTTP server started");
  udp.begin(localPort);
}

void loop()
{
 
  Serial.println("Measuring temperature");
  temp = bmp.readTemperature();
  if (!isnan(temp)) {
    //Serial.println("Temperature:");
    //Serial.println(temp);
    if (!isNight()){ // DAY SETTINGS
      if (temp>eeprom.targetTemp+eeprom.deviation && eeprom.pos>0){
        reverse(512, 20); //Clockwise, close
        eeprom.pos--;
        EEPROM.put(0, eeprom);
        EEPROM.commit();
        Serial.println("Turning clockwise, closing tap");
      }
      else if (temp<eeprom.targetTemp-eeprom.deviation && eeprom.pos<15){
        forward(512, 20); //Counterclockwise, open
        eeprom.pos++;
        EEPROM.put(0, eeprom);
        EEPROM.commit();
        Serial.println("Turning counterclockwise, opening tap");
      }
      printInfo(eeprom.delayTime);
    }  
    else{ // NIGHT SETTINGS
      if (temp>eeprom.nightTargetTemp+eeprom.nightDeviation && eeprom.pos>0){
        reverse(512, 20); //Clockwise, close
        eeprom.pos--;
        EEPROM.put(0, eeprom);
        EEPROM.commit();
        Serial.println("Turning clockwise, closing tap");
      }
      else if (temp<eeprom.nightTargetTemp-eeprom.nightDeviation && eeprom.pos<15){
        forward(512, 20); //Counterclockwise, open
        eeprom.pos++;
        EEPROM.put(0, eeprom);
        EEPROM.commit();
        Serial.println("Turning counterclockwise, opening tap");
      }
      printInfo(eeprom.nightDelayTime);
    }
  }
  else{
    Serial.println("Sensor reading error");
    printInfo(eeprom.delayTime);
  }
}




bool isNight(){
  Serial.println("Asking NTP server");
  WiFi.hostByName(ntpServerName, timeServerIP);
  sendNTPpacket(timeServerIP);
  wait(1000);
  int cb = udp.parsePacket();
  if (!cb) {
    Serial.println("no packet yet");
    while (!cb){
      WiFi.hostByName(ntpServerName, timeServerIP);
      sendNTPpacket(timeServerIP);
      wait(1000);
      cb = udp.parsePacket();
    }
  }
  Serial.print("packet received");
  udp.read(packetBuffer, NTP_PACKET_SIZE);
  unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
  unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
  unsigned long secsSince1900 = highWord << 16 | lowWord;
  const unsigned long seventyYears = 2208988800UL;
  unsigned long epoch = secsSince1900 - seventyYears;
  Serial.println();
  Serial.print("The UTC time is ");
  Serial.print((epoch  % 86400L) / 3600); 
  Serial.print(':');
  Serial.println((epoch  % 3600) / 60);
  if ((epoch  % 86400L) / 3600 + 3 < eeprom.nightStartHour && (epoch  % 86400L) / 3600 + 3 > eeprom.nightEndHour){
    return 0;
  }
  else if ((epoch  % 86400L) / 3600 + 3 == eeprom.nightStartHour && (epoch  % 3600) / 60 < eeprom.nightStartMinute){
    return 0;
  }
  else if ((epoch  % 86400L) / 3600 + 3 == eeprom.nightEndHour && (epoch  % 3600) / 60 >= eeprom.nightEndMinute){
    return 0;
  }
  return 1;
}

void sendNTPpacket(IPAddress& address) {
  Serial.println("sending NTP packet...");
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  udp.beginPacket(address, 123); 
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}

void reverse(int i, int j) {
  // set both motors ON
  digitalWrite(ENA, HIGH);
  digitalWrite(ENB, HIGH);
  while (1)   {
    digitalWrite(IN1, 0);
    digitalWrite(IN2, 1);
    digitalWrite(IN3, 0);
    digitalWrite(IN4, 1);
    motorWait(j);
    i--;
    if (i < 1) break; 
    digitalWrite(IN1, 0);
    digitalWrite(IN2, 1);
    digitalWrite(IN3, 1);
    digitalWrite(IN4, 0);
    motorWait(j);  
    i--;
    if (i < 1) break;
    digitalWrite(IN1, 1);
    digitalWrite(IN2, 0);
    digitalWrite(IN3, 1);
    digitalWrite(IN4, 0);
    motorWait(j);
    i--;
    if (i < 1) break;
    digitalWrite(IN1, 1);
    digitalWrite(IN2, 0);
    digitalWrite(IN3, 0);
    digitalWrite(IN4, 1);
    motorWait(j);  
    i--;
    if (i < 1) break;
  }
  // set both motors OFF
  digitalWrite(ENA, LOW);
  digitalWrite(ENB, LOW);
}  // end reverse()
void forward(int i, int j) {
  // Set both motors ON
  digitalWrite(ENA, HIGH);
  digitalWrite(ENB, HIGH);
  while (1)   {
    digitalWrite(IN1, 0);
    digitalWrite(IN2, 1);
    digitalWrite(IN3, 0);
    digitalWrite(IN4, 1);
    motorWait(j);  
    i--;
    if (i < 1) break;
    digitalWrite(IN1, 1);
    digitalWrite(IN2, 0);
    digitalWrite(IN3, 0);
    digitalWrite(IN4, 1);
    motorWait(j);
    i--;
    if (i < 1) break;
    digitalWrite(IN1, 1);
    digitalWrite(IN2, 0);
    digitalWrite(IN3, 1);
    digitalWrite(IN4, 0);
    motorWait(j);  
    i--;
    if (i < 1) break;
    digitalWrite(IN1, 0);
    digitalWrite(IN2, 1);
    digitalWrite(IN3, 1);
    digitalWrite(IN4, 0);
    motorWait(j);
    i--;
    if (i < 1) break;
  }
  // set both motors OFF
  digitalWrite(ENA, LOW);
  digitalWrite(ENB, LOW);
}  // end forward()

/* Program 3. For BMP280 sensor and with web page where you can change settings.
 * Central water heating system controller for your room.
 *
 * Mautoz Tech https://www.youtube.com/channel/UCWN_KT_CAjGZQG5grHldL8w
 * Заметки ESPшника (Video about this program is there) - https://www.youtube.com/channel/UCQAbEIaWFdARXKqcufV6y_g
 * 
 * Functions: 
 * handleRoot() - starts when somebody requests ESP webpage from web browser and processes request
 * handleSave() - starts when somebody presses "save" on the ESP webpage and saves new settings into permanent memory
 * handleTemp() - sends current parameters to a web browser
 * eepromRead() - reads settings from permanent memory after ESP starts
 * wait(long waitingTime) - it's like delay() function but it doesnt stop webserver
 * motorWait(long waitingTime) - the same but without opportunity to break it via cycleBreak variable
 * printInfo() - sends information via Serial port (speed 9600) between wait function requests.
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
} eeprom;
int outputDelay = 5000;
bool cycleBreak = false;
char htmlResponse[3000];
char htmlResponse2[100];
char htmlResponseTemp[100];
float temp;
const char* ssid = "Wi-Fi";
const char* password = "Wi-Fi Password";

void handleRoot() {
  snprintf ( htmlResponse, 3000,
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
            Delay between measurements<input type='text' value=           id='setDelayTime' size=3 autofocus>\
            <div>\<br><button id=\"send_button\">Save</button>\
            </div>\
            <div id=\'receivedMessageFromServer\'>\
            </div>\
      <script src=\"https://ajax.googleapis.com/ajax/libs/jquery/1.11.3/jquery.min.js\"></script>\    
      <script>\
        $('#send_button').click(function(e){\
          var targetTemp;\
          var deviation;\
          var delayTime;\
          e.preventDefault();\
          targetTemp = $('#setTargetTemp').val();\
          deviation = $('#setDeviation').val();\
          delayTime = $('#setDelayTime').val();\        
          $.get('/save?targetTemp=' + targetTemp + '&deviation=' + deviation + '&delayTime=' + delayTime, function(data){\
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
  
  String myTempStr = "'"+String(eeprom.targetTemp)+"'";
  String deviationStr = "'"+String(eeprom.deviation)+"'";
  String delayTimeStr = "'"+String(eeprom.delayTime)+"'";
  for (int i=0; i<myTempStr.length(); i++){
    htmlResponse[319+66+i] = myTempStr[i];
  }
  for (int i=0; i<deviationStr.length(); i++){
    htmlResponse[413+66+i] = deviationStr[i];
  }
  for (int i=0; i<delayTimeStr.length(); i++){
    htmlResponse[504+66+i] = delayTimeStr[i];
  }
  //Serial.println(htmlResponse);
  server.send (200, "text/html", htmlResponse);  
  Serial.println("HANDLING ROOT");
}

void handleSave() {
  if (server.args()==3){
    if (server.arg("deviation")!=""){
      float receivedDeviation = server.arg("deviation").toFloat();
      if (receivedDeviation>0.1 && receivedDeviation<2){
        eeprom.deviation = receivedDeviation;
      }  
    }
    if (server.arg("delayTime")!=""){
      long receivedDelayTime = server.arg("delayTime").toInt();
      if (receivedDelayTime>1000 && receivedDelayTime<7200000){
        eeprom.delayTime = receivedDelayTime;
      }  
    }
    if (server.arg("targetTemp")!=""){
      float receivedtargetTemp = server.arg("targetTemp").toFloat();
      if (receivedtargetTemp>20 && receivedtargetTemp<30){
        eeprom.targetTemp = receivedtargetTemp;
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
  server.send (200, "text/html", "Temperature:"+String(temp)+" Position:"+String(eeprom.pos)+" TargetTemperature:"+String(eeprom.targetTemp)+" Deviation:"+String(eeprom.deviation)+" DelayTime:"+String(eeprom.delayTime)); 
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
    eeprom.pos = 0; //0
    eeprom.targetTemp = 24.5; //25.5;
    eeprom.deviation = 0.25; //1
    eeprom.delayTime = 300;
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

void printInfo(){
  float cycleTimes = eeprom.delayTime / outputDelay;
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
  server.begin();
  Serial.println("HTTP server started");
  
}

void loop()
{
  temp = bmp.readTemperature();
  if (!isnan(temp)) {
    //Serial.println("Temperature:");
    //Serial.println(temp);
    Serial.println("Measuring temperature");
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
  }
  else{
    Serial.println("Sensor reading error");
  }
  printInfo();
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

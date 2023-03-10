/* Program 1. For DHT11 sensor and without web-server.
 * Central water heating system controller for your room.
 *
 * Functions "reverse" and "forward" rotate the motor. Model: 28BYJ-48
 * 
 * Used components: 
 * Microcontroller: ESP8266 NodeMCU,
 * Motor: 28BUJ-48, 
 * Motor controller shield: L298N, 
 * Temperature Sensor: DHT11
 * 
 * Mautoz Tech https://www.youtube.com/channel/UCWN_KT_CAjGZQG5grHldL8w
 * Заметки ESPшника (Video about this program is there) - https://www.youtube.com/channel/UCQAbEIaWFdARXKqcufV6y_g
 * Licensed under GNU GPL V3: This program is a free software, you are free to use or modify the code. No warranty.
 */

#include "DHT.h"
#include <ESP_EEPROM.h>
#define DHTPIN 2 // DHT PIN
DHT dht(DHTPIN, DHT11);
const int ENA = 13;
const int IN1 = 12;
const int IN2 = 14;
const int IN4 = 0;
const int IN3 = 4;
const int ENB = 5;

struct Settings{
  int pos;
  float targetTemp;
  float deviation;
  long delayTime;
} eeprom;


void setup()
{
  Serial.begin(9600);
  EEPROM.begin(sizeof(Settings));
  dht.begin();
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

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
    eeprom.pos = 0;
    eeprom.targetTemp = 25; 
    eeprom.deviation = 0.25;
    eeprom.delayTime = 300;
    Serial.println("Applying default settings");
  }
}

void loop()
{
  float temp = dht.readTemperature();
  if (!isnan(temp)) {
    Serial.println("Measuring temperature");
    if (temp>eeprom.targetTemp+eeprom.deviation && eeprom.pos>0){
      reverse(512, 20);
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
  for (int i = 0; i<20; i++){
    float temp = dht.readTemperature();
    Serial.println("Temperature:");
    Serial.println(temp);
    Serial.println("Position:");
    Serial.println(eeprom.pos);
    delay(eeprom.delayTime*50);
  }  
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
    delay(j);
    i--;
    if (i < 1) break; 
    digitalWrite(IN1, 0);
    digitalWrite(IN2, 1);
    digitalWrite(IN3, 1);
    digitalWrite(IN4, 0);
    delay(j);  
    i--;
    if (i < 1) break;
    digitalWrite(IN1, 1);
    digitalWrite(IN2, 0);
    digitalWrite(IN3, 1);
    digitalWrite(IN4, 0);
    delay(j);
    i--;
    if (i < 1) break;
    digitalWrite(IN1, 1);
    digitalWrite(IN2, 0);
    digitalWrite(IN3, 0);
    digitalWrite(IN4, 1);
    delay(j);  
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
    delay(j);  
    i--;
    if (i < 1) break;
    digitalWrite(IN1, 1);
    digitalWrite(IN2, 0);
    digitalWrite(IN3, 0);
    digitalWrite(IN4, 1);
    delay(j);
    i--;
    if (i < 1) break;
    digitalWrite(IN1, 1);
    digitalWrite(IN2, 0);
    digitalWrite(IN3, 1);
    digitalWrite(IN4, 0);
    delay(j);  
    i--;
    if (i < 1) break;
    digitalWrite(IN1, 0);
    digitalWrite(IN2, 1);
    digitalWrite(IN3, 1);
    digitalWrite(IN4, 0);
    delay(j);
    i--;
    if (i < 1) break;
  }
  // set both motors OFF
  digitalWrite(ENA, LOW);
  digitalWrite(ENB, LOW);
}  // end forward()

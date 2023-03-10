/*
 * Driving a 5V stepper motor using Keyes L298N Dual Motor Driver;
 * Chienline @2015;
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

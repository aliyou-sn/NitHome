#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <analogWrite.h>
#include "DHT.h"
#define DHTPIN 13
#define timeSeconds 10
#define BLYNK_TEMPLATE_ID "TMPL2y3hal-kS"
#define BLYNK_TEMPLATE_NAME "NitHome"
#define BLYNK_AUTH_TOKEN "3BrCtyfozd9DnO5G1HeKAb3ecDzyM7Vi"
#define TRIGPIN    32  
#define ECHOPIN    35  
#define BuzzerPin  5
#define FullLed   14  
#define LowLed   26  
#define pir 27
#define light 2
#define pump 25
// #define SCREEN_WIDTH 128 // OLED display width, in pixels
// #define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define TankLevel    V0
#define TankDistance    V1
#define Waterpump    V2
#define LightState    V3
#define MotionState    V4
#define TempState    V5
#define DHTTYPE DHT11
#define ldr 36
#define StreetLight 15
// #define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
// Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
DHT dht(DHTPIN, DHTTYPE);

char ssid[] = "Aliyu";
char pass[] = "macbooker";
float temp;
float duration;
float distance;
int   waterLevelPer;
const int EmptyTank = 120;
const int FullTank = 30;
int triggerPer =   10;
int LightInit;
int LightValue;


unsigned long now = millis();
unsigned long lastTrigger = 0;
boolean startTimer = false;
boolean motion = false;

// void displayData(int value){
//   display.clearDisplay();
//   display.setTextSize(4);
//   display.setCursor(8,2);
//   display.print(value);
//   display.print(" ");
//   display.print("%");
//   display.display();
// }
BlynkTimer timer; 
BLYNK_CONNECTED() {
  Blynk.syncVirtual(TankLevel);
  Blynk.syncVirtual(TankDistance);
  Blynk.syncVirtual(Waterpump);
  Blynk.syncVirtual(LightState);
  Blynk.syncVirtual(MotionState);
  Blynk.syncVirtual(TempState);
}

void myTimerEvent()
{
  Blynk.virtualWrite(V0, millis() / 1000);
  Blynk.virtualWrite(V1, millis() / 1000);
  Blynk.virtualWrite(V2, millis() / 1000);
  Blynk.virtualWrite(V3, millis() / 1000);
  Blynk.virtualWrite(V4, millis() / 1000);
  Blynk.virtualWrite(V5, millis() / 1000);
}


void PumpControl(void *parameter){
  for(;;){
  digitalWrite(TRIGPIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGPIN, HIGH);
  delayMicroseconds(20);
  digitalWrite(TRIGPIN, LOW);
  duration = pulseIn(ECHOPIN, HIGH);
  distance = ((duration / 2) * 0.343);
  if (distance >= FullTank && distance <= EmptyTank ){
    waterLevelPer = map((int)distance ,EmptyTank, FullTank, 0, 100);
    // displayData(waterLevelPer);
    Blynk.virtualWrite(TankLevel, waterLevelPer);
    Blynk.virtualWrite(TankDistance, (String(distance) + " mm"));
    if(waterLevelPer < 20){
      digitalWrite(LowLed, HIGH);
      digitalWrite(FullLed,LOW);
    }
    if(waterLevelPer > 50 && waterLevelPer < 100){
      digitalWrite(FullLed,HIGH);
      digitalWrite(LowLed,LOW);
    }
    
    if (waterLevelPer > triggerPer){
      digitalWrite(LowLed, LOW);
      } 
} 
  if (distance >= EmptyTank){
      digitalWrite(pump,HIGH);
      Blynk.virtualWrite(Waterpump, "ON");
      }
    
  if (distance <= FullTank){
    digitalWrite(pump,LOW);
    Blynk.virtualWrite(Waterpump, "OFF");
  }  
  
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" mm");
  }

  vTaskDelay(200 / portTICK_PERIOD_MS);
}

void IRAM_ATTR detectsMovement() {
  digitalWrite(light, HIGH);
  startTimer = true;
  lastTrigger = millis();
}

void MeasureTemp(void *parameter){
  for(;;){
  temp = dht.readTemperature();
  Blynk.virtualWrite(TempState, temp);
  Serial.print("Temperature : ");
  Serial.println(temp);
  vTaskDelay(500 / portTICK_PERIOD_MS);
  
  }
}

// void AutoLight(void *parameter){
//   for(;;){
//   LightValue = analogRead(ldr);
//   if(LightValue-LightInit < 50){
//     digitalWrite(StreetLight,HIGH);
//   }
//   else{
//     digitalWrite(StreetLight, LOW);
//   }
//   }
// }



void setup()
{
  // Debug console
  Serial.begin(115200);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  timer.setInterval(1000L, myTimerEvent);
  dht.begin();
  xTaskCreate(PumpControl,"Pump comtro1", 5000, NULL, 2, NULL);
  xTaskCreate(MeasureTemp,"Temperature ", 8000, NULL, 3, NULL);
  // xTaskCreate(AutoLight,"Auto light ", 5000, NULL, 3, NULL);
  attachInterrupt(digitalPinToInterrupt(pir), detectsMovement, RISING);
  pinMode(pir, INPUT_PULLUP);
  pinMode(ECHOPIN, INPUT);
  pinMode(TRIGPIN, OUTPUT);
  pinMode(FullLed, OUTPUT);
  pinMode(LowLed, OUTPUT);
  pinMode(BuzzerPin, OUTPUT);
  pinMode(pump, OUTPUT);
  digitalWrite(BuzzerPin, LOW);
  pinMode(light, OUTPUT);
  pinMode(StreetLight, OUTPUT);
  digitalWrite(light, LOW);
  // if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
  //   Serial.println(F("SSD1306 allocation failed"));
  //   for(;;);
  // }
  // delay(1000);  
  // display.setTextSize(1);
  // display.setTextColor(WHITE);
  // display.clearDisplay();
  LightInit = analogRead(ldr);
  
}

void loop()
{
  // MeasureTemp();
  // PumpControl();
  Blynk.run();
  timer.run();
  now = millis();
  if((digitalRead(light) == HIGH) && (motion == false)) {
    Serial.println("MOTION DETECTED!!!");
    Blynk.virtualWrite(MotionState, "MOTION DETECTED!!!");
    Blynk.virtualWrite(LightState, "ON");

    motion = true;
  }
  if(startTimer && (now - lastTrigger > (timeSeconds*1000))) {
    digitalWrite(light, LOW);
    Serial.println("Motion stopped...");
    Blynk.virtualWrite(MotionState, "Motion stopped...");
    Blynk.virtualWrite(LightState, "OFF");
    startTimer = false;
    motion = false;

  }
}

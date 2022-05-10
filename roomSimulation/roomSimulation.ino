#define timeSeconds 10 //motion sensor timer

// Fill-in information from your Blynk Template here
#define BLYNK_TEMPLATE_ID "TMPLpiM5SPEw"
#define BLYNK_DEVICE_NAME "LED and Fan 1"

#define BLYNK_FIRMWARE_VERSION        "0.1.0"

#define BLYNK_PRINT Serial
//#define BLYNK_DEBUG

#define APP_DEBUG
#define rainAnalog 35

// Uncomment your board, or configure a custom board in Settings.h
//#define USE_WROVER_BOARD
//#define USE_TTGO_T7

#include "BlynkEdgent.h"
const int led = 26;
const int motionSensor = 27;
const int INA = 4;
const int INB = 5;
int pirValue;
int pirState = LOW;
BlynkTimer timer1;

int motion;
int fan;
int light;
int timer;
int rainAnalogVal;

// Timer: Auxiliary variables
unsigned long now = millis();
unsigned long lastTrigger = 0;
boolean startTimer = false;



//byte speed = 80; // change this to alter speed 0 - 255 although 80 is the minimum for reliable operation

//void reverse(){
//  digitalWrite(INA,0);
//  digitalWrite(INB,speed);
//}
//
//void forward(){
//  Serial.println("test1");
//  digitalWrite(INA,speed);
//  digitalWrite(INB,0);
//}
//
//void stopFan(){
//  digitalWrite(INA,LOW);
//  digitalWrite(INB,LOW);
//}

//
//void IRAM_ATTR detectsMovement() {
//  if (motion == 1){
//    Serial.println("MOTION DETECTED!!!");
//    digitalWrite(led, HIGH);
//    digitalWrite(INA,0);
//    digitalWrite(INB, HIGH);
//    startTimer = true;
//    lastTrigger = millis();
//  }
//  
//}


void setup()
{
  Serial.begin(115200);
  delay(100);

  // PIR Motion Sensor mode INPUT_PULLUP
  pinMode(motionSensor, INPUT);
  // Set motionSensor pin as interrupt, assign interrupt function and set RISING mode
  //attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);

  //for fan
  pinMode(INA,OUTPUT);
  pinMode(INB,OUTPUT);
  // Set LED to LOW
  pinMode(led, OUTPUT);
  //digitalWrite(led, LOW);
  pinMode(rainAnalog, INPUT);
  timer1.setInterval(1000L, myTimerEvent);

  BlynkEdgent.begin();
}

void loop() {
  now = millis();

  if(fan == 1)
  {
    Fan(1);
  }
  if(fan == 0)
  {
    Fan(0);
  }
  if(light == 1)
  {
    Light(1);
  }
  if(light == 0)
  {
    Light(0);
  }
  if(motion == 1)
  {
    getPirValue();
  }
  timer1.run();
  BlynkEdgent.run();

}

void getPirValue(void)
{
  pirValue = digitalRead(motionSensor);
  //Serial.println(pirValue);
  if (pirValue == HIGH) 
  { 
    Light(1);
    Fan(1);
    startTimer = true;
    lastTrigger = millis();
    if(pirState == LOW)//check whether we turn off
    {
      pirState = HIGH; //to indicate we just turn on
      Serial.println("Motion detected!");
    }
  }
  else
  {
    if(pirState == HIGH && (startTimer &&(now-lastTrigger > (timer*1000)))) //if we previously turn on and timer runs out without refreshing from pirValue
    {
      Light(0);
      Fan(0);
      pirState = LOW; //to indicate we just turn off
      startTimer = false;
      Serial.println("Motion ended!");
    }
  }
  
  if(now-lastTrigger < (timer*1000))
  {
    Serial.println(now-lastTrigger);
    Light(1);
    Fan(1);
  }
  
}

void Fan(int value)
{
  if(value ==1)
  {
    digitalWrite(INA,LOW);
    digitalWrite(INB,HIGH);
  }
  else
  {
    digitalWrite(INA,LOW);
    digitalWrite(INB,LOW); 
  }

}

void myTimerEvent()
{
  rainAnalogVal = analogRead(rainAnalog);
  Blynk.virtualWrite(V5, rainAnalogVal);
}


void Light(int value)
{
  digitalWrite(led, value);
}

BLYNK_WRITE(V27)//MOTION SENSOR maybe i can disable and enable the motion sensor itself?
{
  motion = param.asInt();
}

BLYNK_WRITE(V26)//LED
{
  light  = param.asInt();  
}

BLYNK_WRITE(V4)//FAN
{
  fan = param.asInt();
}

BLYNK_WRITE(V1)//Motion Timer
{
  timer = param.asInt();
}
//BLYNK_READ(V5)
//{
//  rainAnalogVal = analogRead(rainAnalog);
//  Serial.println(rainAnalogVal);
//  Blynk.virtualWrite(V5,(4095-rainAnalogVal));
//}

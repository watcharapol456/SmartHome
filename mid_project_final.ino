#define BLYNK_TEMPLATE_ID "TMPL6Es84cSeS"
#define BLYNK_TEMPLATE_NAME "smarthome"
#define BLYNK_AUTH_TOKEN "vH432KXmqoAv93OsmO8ji0xzKh437zT4"

#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Servo.h>

const int PIRPin = D0;
const int LightPin = D1;
const int PlugPin = D2;
Servo myservo;
#define TRIGGER_PIN D5
#define ECHO_PIN D6

int PIRValue;
int statusDoor;
char ssid[] = "iPhone.";
char pass[] = "987654321";

bool isV7Pressed = false;

long radarUltrasonic() {
  long duration, distance;
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);
  duration = pulseIn(ECHO_PIN, HIGH);
  distance = (duration / 2) / 29.1;
  return distance;
}

BlynkTimer timer;
int lightState = 0;
int plugState = 0;
bool motionDetected = false;
unsigned long lastMotionTime = 0;
const unsigned long debounceDelay = 1000; // 1 second debounce delay

void setLight(int value) {
  digitalWrite(LightPin, value);
  lightState = value;
  Blynk.virtualWrite(V2, lightState);
}

void setPlug(int valuep) {
  digitalWrite(PlugPin, valuep);
  plugState = valuep;
  Blynk.virtualWrite(V3, plugState);
}

BLYNK_WRITE(V0) {
  if (isV7Pressed) {
    int value = param.asInt();
    Serial.println(value);
    setLight(value);
  } else {
    Serial.println("V7 button is not pressed, V0 is inactive.");
  }
}

BLYNK_WRITE(V1) {
  if (isV7Pressed) {
    int valuep = param.asInt();
    Serial.println(valuep);
    setPlug(valuep);
  } else {
    Serial.println("V7 button is not pressed, V1 is inactive.");
  }
}

BLYNK_WRITE(V7) {
  isV7Pressed = param.asInt();
  Serial.print("V7 button state: ");
  Serial.println(isV7Pressed);
}

BLYNK_WRITE(V8) {
  if (isV7Pressed) {
    int valued = param.asInt();
    Serial.println(valued);
    if (valued == 1) {
      myservo.write(180);
      Serial.println("Servo moved to 180 degrees");
      statusDoor = 1;
      Blynk.virtualWrite(V6, statusDoor);
    } else if (valued == 0) {
      myservo.write(0);
      Serial.println("Servo moved to 0 degrees");
      statusDoor = 0;
      Blynk.virtualWrite(V6, statusDoor);
    }
  } else {
    Serial.println("V7 button is not pressed, V8 is inactive.");
  }
}

BLYNK_CONNECTED() {
  Blynk.setProperty(V3, "offImageUrl", "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations.png");
  Blynk.setProperty(V3, "onImageUrl", "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations_pressed.png");
  Blynk.setProperty(V3, "url", "https://docs.blynk.io/en/getting-started/what-do-i-need-to-blynk/how-quickstart-device-was-made");
}

int motionCount = 0;
int leaveCount = 0;

void myTimerEvent() {
  if (!isV7Pressed) {
    long distance_cm = radarUltrasonic();
    Serial.print(distance_cm);
    Serial.println(" cm.");
    PIRValue = digitalRead(PIRPin);
    Serial.print("PIRValue: ");
    Serial.println(PIRValue);
    Blynk.virtualWrite(V4, distance_cm);


    if (distance_cm <= 100) {
      myservo.write(180);
      statusDoor = 1;
      Blynk.virtualWrite(V6, statusDoor);
      delay(1000);
    } else {
      myservo.write(0);
      statusDoor = 0;
      Blynk.virtualWrite(V6, statusDoor);
      delay(1000);
    }

    // Debounce the PIR sensor input
    unsigned long currentTime = millis();
    if (PIRValue == HIGH && !motionDetected) {
      if (currentTime - lastMotionTime > debounceDelay) {
        motionCount++;
        motionDetected = true;
        lastMotionTime = currentTime;
        Serial.print("motionCount+: ");
        Serial.println(motionCount);
      }
    } else if (PIRValue == LOW && motionDetected) {
      if (currentTime - lastMotionTime > debounceDelay) {
        motionDetected = false;
        lastMotionTime = currentTime;
      }
    }

    leaveCount++;
    if (leaveCount >= 10) {
      if (PIRValue == LOW && motionCount != 0) {
        motionCount--;
        Serial.print("motionCount-: ");
        Serial.println(motionCount);
      }
      leaveCount = 0;
    }

    if (motionCount > 0) {
      digitalWrite(LightPin, HIGH);
      Blynk.virtualWrite(V2, lightState);
    } else {
      digitalWrite(LightPin, LOW);
      Blynk.virtualWrite(V2, lightState);
    }

       Blynk.virtualWrite(V9, motionCount);
       Blynk.virtualWrite(V4, distance_cm);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIRPin, INPUT);
  pinMode(LightPin, OUTPUT);
  pinMode(PlugPin, OUTPUT);
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  myservo.attach(D3);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  timer.setInterval(1000L, myTimerEvent);
}

void loop() {
  Blynk.run();
  timer.run();
}

#include <Servo.h>
#include "DHT.h"
#include <SoftwareSerial.h>

#define DHTPIN1 2    
#define DHTPIN2 3     
#define DHTTYPE DHT11   // DHT 11 

DHT dht1(DHTPIN1, DHTTYPE);
DHT dht2(DHTPIN2, DHTTYPE);

Servo myServo;

const int actuators[] = {13, 12, 11}; // 0: electricity, 1: water pump, 2: air fan
const int smkPin = A0; 
const int flmPin = 4;
const int servoPin = 5;

SoftwareSerial BTSerial(2, 3); // RX, TX for HC-06

bool prevFireState = false;
bool fireState = false;
float tempDifference = 0.0;
int airQuality = 0;

void setup() {
  pinMode(flmPin, INPUT);
  pinMode(9, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(6, OUTPUT);
  myServo.attach(servoPin);
  for (int i = 0; i < 3; i++) {
    pinMode(actuators[i], OUTPUT);
    Serial.begin(9600);
    dht1.begin();
    dht2.begin();
  }
  BTSerial.begin(9600); // Start Bluetooth serial communication
}

void loop() {
  int smkValue = map(analogRead(smkPin), 0, 1024, 0, 255);
  int flmValue = digitalRead(flmPin); 

  delay(100);

  digitalWrite(9, 1);

  for (int angle = 0; angle <= 180; angle += 1) {
    if (flmValue == 0) {
      fireState = true;
      digitalWrite(actuators[0], 0);
      digitalWrite(actuators[1], 1);
      Serial.println("flame detected");
      delay(100);
    } else {
      fireState = false;
      Serial.println("No flame");
      delay(100);
      digitalWrite(actuators[0], 1);
      digitalWrite(actuators[1], 0);
    }
    if (smkValue >= 50) {
      airQuality = smkValue;
      digitalWrite(actuators[2], 1);
      Serial.println(smkValue);
    } else {  
      airQuality = smkValue;
      digitalWrite(actuators[2], 0);
      Serial.println(smkValue);
    }

    myServo.write(angle);
    delay(15);

    float humidity1 = dht1.readHumidity();
    float temperature1 = dht1.readTemperature();

    float humidity2 = dht2.readHumidity();
    float temperature2 = dht2.readTemperature();

    if (isnan(humidity1) || isnan(temperature1) || isnan(humidity2) || isnan(temperature2)) {
      Serial.println("Failed to read from one or both DHT sensors!");
      return;
    }
    delay(500);
    Serial.print("Sensor 1 - Humidity: ");
    Serial.print(humidity1);
    Serial.print("%  Temperature: ");
    Serial.print(temperature1);
    Serial.println("°C ");

    Serial.print("Sensor 2 - Humidity: ");
    Serial.print(humidity2);
    Serial.print("%  Temperature: ");
    Serial.print(temperature2);
    Serial.println("°C ");
    Serial.print("The result :");
    tempDifference = temperature1 - temperature2;
    Serial.println(tempDifference);
  }

  for (int angle = 180; angle >= 0; angle -= 1) {
    myServo.write(angle);
    delay(15);  
  }

  // Send notification when fire is detected
  if (fireState && !prevFireState) {
    sendNotification("Fire is still on !");
  }
  
  // Send notification when fire is out
  if (!fireState && prevFireState) {
    sendNotification("Fire extinguished.");
  }

  prevFireState = fireState; // Update previous fire state
}

void sendNotification(String message) {
  // Construct the notification message
  String fullMessage = "Bluetooth: connected, " + message + " Temp Diff: ";
  fullMessage += String(tempDifference, 1); // Append temperature difference
  fullMessage += "°C, Air Quality: ";
  fullMessage += airQuality;
  
  // Send the message via Bluetooth
  BTSerial.print(fullMessage);
}

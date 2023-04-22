#include <Wire.h>
#include <MPU6050.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>

MPU6050 mpu;
TinyGPS gps;

const char* phoneNumber = "+359877841411"; // change this to add my emergency contact's number
const char* phoneNumber112 = "359877841411"; // change this to 112 (for the example just other phone)

const float crashThresholdX = 1.0; //has to be 20-30 for actual car crash
const float crashThresholdY = 1.0;
const float crashThresholdZ = 2.0;
int mqPin=A0;
int mqData;
int neoRXPin = 6;
int neoTXPin = 7;
int gsmRxPin = 11;
int gsmTxPin = 10;
int buzzerPin = 12;

SoftwareSerial gpsSerial(neoRXPin, neoTXPin);
SoftwareSerial gsmSerial(gsmRxPin, gsmTxPin);

void setup() {
  Serial.begin(9600);
  Wire.begin();
  mpu.initialize();
  pinMode(mqPin,INPUT); 
  gpsSerial.begin(9600);
  gsmSerial.begin(9600);
  pinMode(buzzerPin, OUTPUT);

}

void loop() {
  int16_t accelX, accelY, accelZ;
  mpu.getAcceleration(&accelX, &accelY, &accelZ);

  float accelX_g = accelX / 16384.0;
  float accelY_g = accelY / 16384.0;
  float accelZ_g = accelZ / 16384.0;

  if (abs(accelX_g) > crashThresholdX || abs(accelY_g) > crashThresholdY || abs(accelZ_g) > crashThresholdZ) {
    digitalWrite(buzzerPin, HIGH);
    while (gpsSerial.available() > 0) {

      if (gps.encode(gpsSerial.read())) {
        float latitude, longitude;
        gps.f_get_position(&latitude, &longitude);

        // send the SMS with the GPS coordinates
        sendSMS(phoneNumber, "Crash detected! Location: http://maps.google.com/?q=" + String(latitude, 6) + "," + String(longitude, 6));
        // make a call to the same number
        //makeCall(phoneNumber);

        mqData = analogRead(mqPin);  
        if(mqData > 280) {
            sendSMS(phoneNumber112, "Fire detected at Location: http://maps.google.com/?q=" + String(latitude, 6) + "," + String(longitude, 6));
            //makeCall(phoneNumber112);
        } 

        delay(100);
      }
    }
  }
}

void sendSMS(const char* phoneNumber, const String& message) {
  gsmSerial.println("AT+CMGF=1"); // set SMS mode to text
  delay(100);
  gsmSerial.println("AT+CMGS=\"" + String(phoneNumber) + "\""); // set the phone number
  delay(100);
  gsmSerial.println(message); // set the message
  delay(100);
  gsmSerial.write(26); // send the message
}

void makeCall(const char* phoneNumber) {
  gsmSerial.println("ATD" + String(phoneNumber) + ";"); // dial the phone number
  delay(5000); // wait for 5 seconds
  gsmSerial.println("ATH"); // hang up the call
}
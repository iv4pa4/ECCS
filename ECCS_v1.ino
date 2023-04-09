#include <Wire.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>

#include "MPU6050.h"

#define IMPACT_THRESHOLD 10 // adjust 
#define SMOKE_THRESHOLD 500   // adjust

//adjust
#define GPS_RX_PIN 6
#define GPS_TX_PIN 7
#define GSM_RX_PIN 11
#define GSM_TX_PIN 10
#define BUZZER_PIN 12
#define SMOKE_SENSOR_PIN A0

SoftwareSerial gpsSerial(GPS_RX_PIN, GPS_TX_PIN);
SoftwareSerial gsmSerial(GSM_RX_PIN, GSM_TX_PIN);

MPU6050 mpu;

TinyGPS gps;

const char* phoneNumber = "+359877841411"; // change this to my phone number
const char* phoneNumber112 = "1234567890"; // change this to 112 (for the example just other phone)

void setup() {
  Serial.begin(9600);
  gpsSerial.begin(9600);
  gsmSerial.begin(9600);
  pinMode(BUZZER_PIN, OUTPUT);
}

void loop() {
  int16_t ax, ay, az, gx, gy, gz;
  double latitude = 0, longitude = 0;
  float smokeLevel = analogRead(SMOKE_SENSOR_PIN);

  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  float acceleration = sqrt(ax * ax + ay * ay + az * az);

  // detect impact
  if (acceleration > IMPACT_THRESHOLD) {
    // get the GPS coordinates
    while (gpsSerial.available() > 0) {
      if (gps.encode(gpsSerial.read())) {
        float latitude, longitude;
        gps.f_get_position(&latitude, &longitude);

        // send the SMS with the GPS coordinates
        sendSMS(phoneNumber, "Crash detected! Location: http://maps.google.com/?q=" + String(latitude, 6) + "," + String(longitude, 6));

        // make a call to the same number
        makeCall(phoneNumber);

        // start the buzzer and smoke sensor
        digitalWrite(BUZZER_PIN, HIGH);
        if (smokeLevel > SMOKE_THRESHOLD) {
          sendSMS(phoneNumber112, "Smoke detected at Location: http://maps.google.com/?q=" + String(latitude, 6) + "," + String(longitude, 6));
          makeCall(phoneNumber112);
        }

        // wait for 5 seconds before turning off the buzzer and smoke sensor
        delay(5000);
        digitalWrite(BUZZER_PIN, LOW);
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


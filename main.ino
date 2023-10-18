#include <PMserial.h>
SerialPM pms(PMSx003, 32, 27);  // PMSx003, RX, TX

int redPin= 14;
int greenPin = 12;
int bluePin = 13;

// setting PWM properties
const int freq = 5000;
const int ledChannel = 0;
const int resolution = 8;

void setup() {
  Serial.begin(9600);
  pms.init();                   // config serial port

  ledcSetup(ledChannel, freq, resolution);
  ledcAttachPin(greenPin, ledChannel);
  ledcAttachPin(redPin, ledChannel);
  ledcAttachPin(bluePin, ledChannel);
    
}

void loop() {
  pms.read();                   // read the PM sensor
  Serial.print(F("PM1.0 "));Serial.print(pms.pm01);Serial.print(F(", "));
  Serial.print(F("PM2.5 "));Serial.print(pms.pm25);Serial.print(F(", "));
  Serial.print(F("PM10 ")) ;Serial.print(pms.pm10);Serial.println(F(" [ug/m3]"));
  delay(500);                 // wait for 10 seconds

  // if pm10 is below 10, set led to green,
    // if pm10 is between 10 and 20, set led to yellow,
    // if pm10 is above 20, set led to red

  if (pms.pm10 < 10) {
    ledcWrite(redPin, 3);
    ledcWrite(greenPin, 252);
    ledcWrite(bluePin, 48);
  } else if (pms.pm10 < 20) {
    ledcWrite(redPin, 252);
    ledcWrite(greenPin, 202);
    ledcWrite(bluePin, 3);
  } else {
    ledcWrite(redPin, 252);
    ledcWrite(greenPin, 3);
    ledcWrite(bluePin, 34);
  }
  
}

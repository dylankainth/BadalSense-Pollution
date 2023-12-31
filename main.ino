#include <SoftwareSerial.h>
#include <MHZ.h>
#include <PMserial.h>

// pin for pwm reading
#define CO2_IN 10

// pin for uart reading
#define MH_Z19_RX 17  // D7
#define MH_Z19_TX 16  // D6

SerialPM pms(PMSx003, 32, 27);  // PMSx003, RX, TX
MHZ co2(MH_Z19_RX, MH_Z19_TX, CO2_IN, MHZ19B);

void setup() {
  Serial.begin(9600);
  pinMode(CO2_IN, INPUT);
  pms.init();  
  delay(100);
  Serial.println("MHZ 19B");

  // enable debug to get addition information
  // co2.setDebug(true);

  if (co2.isPreHeating()) {
    Serial.print("Preheating");
    while (co2.isPreHeating()) {
      Serial.print(".");
      delay(5000);
    }
    Serial.println();
  }
}

void loop() {
  pms.read(); 
  Serial.print(F("PM1.0 "));Serial.print(pms.pm01);Serial.print(F(", "));
  Serial.print(F("PM2.5 "));Serial.print(pms.pm25);Serial.print(F(", "));
  Serial.print(F("PM10 ")) ;Serial.print(pms.pm10);Serial.println(F(" [ug/m3]"));

  Serial.print("\n----- Time from start: ");
  Serial.print(millis() / 1000);
  Serial.println(" s");

  int ppm_uart = co2.readCO2UART();
  Serial.print("PPMuart: ");

  if (ppm_uart > 0) {
    Serial.print(ppm_uart);
  } else {
    Serial.print("n/a");
  }

  delay(5000);
}
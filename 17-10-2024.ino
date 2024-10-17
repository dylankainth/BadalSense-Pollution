#include <SoftwareSerial.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <PMserial.h>

struct CO2Output
{
     int ppm_uart;
     int temp;
};

struct PMOutput
{
  int pm01;
  int pm25;
  int pm10;
};

byte getCheckSum(byte *packet) {
    byte i;
    unsigned char checksum = 0;
    for (i = 1; i < 8; i++) {
        checksum += packet[i];
    }
    checksum = 0xff - checksum;
    checksum += 1;
    return checksum;
}

CO2Output getCO2Reading() {
  SoftwareSerial co2Serial(17,16);
  co2Serial.begin(9600);

  byte CO2Command[9] = { 0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79 };  // Add semicolon here
  byte CO2Response[9];

  co2Serial.write(CO2Command, 9);

  memset(CO2Response, 0, 9); // clear data register for co2 reader
  int i = 0;
  while (co2Serial.available() == 0) {
    delay(1000);
    i++;
    Serial.println("CO2: Waiting for sensor to come online");
  }

  if (co2Serial.available() > 0) {
    co2Serial.readBytes(CO2Response, 9);
  }

  byte CO2Check = getCheckSum(CO2Response);
  if (CO2Response[8] != CO2Check) { 
    Serial.println("CO2: Checksum not OK!");
    Serial.print("Received: ");
    Serial.print(CO2Response[8]);
    Serial.print("; Should be: ");
    Serial.print(CO2Check);
  }

  int ppm_uart = 256 * (int)CO2Response[2] + CO2Response[3];
  byte temp = CO2Response[4] - 40;
  byte status = CO2Response[5];

  Serial.println("CO2: " + String(ppm_uart) + "," + String(temp));
  
  CO2Output result = {ppm_uart, temp};
  return result;
}

PMOutput getPMReading() { 
  SerialPM pms(PMSx003, 32, 27);

  pms.init();

  pms.read();

  PMOutput result = {pms.pm01, pms.pm25, pms.pm10};
  return result;
}

void setup() {
  Serial.begin(9600);
  delay(1000); // allow some time for serial to start up 

  CO2Output CO2Data;
  PMOutput PMSData;

  CO2Data = getCO2Reading();
  PMSData = getPMReading();
}



void loop() {

}
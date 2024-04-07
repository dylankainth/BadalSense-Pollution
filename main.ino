#include <SoftwareSerial.h>
#include <WiFi.h>
#include <HTTPClient.h>

SoftwareSerial co2Serial(17, 16); // define MH-Z19 RX TX
unsigned long startTime = millis();
#include <PMserial.h>
SerialPM pms(PMSx003, 32, 27);  // PMSx003, RX, TX

const char* ssid = "xx";
const char* password = "x";
const char* serverName = "xxx";
//const char* serverName = "badalsense-pollution.netlify.app/.netlify/functions/input";
int noX = 0;

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  30        /* Time ESP32 will go to sleep (in seconds) */

RTC_DATA_ATTR int bootCount = 0;

/*
Method to print the reason by which ESP32
has been awaken from sleep
*/
void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}


void setup() {
    Serial.begin(9600);
    delay(1000); //Take some time to open up the Serial Monitor

    ++bootCount;
    Serial.println("Boot number: " + String(bootCount));

    print_wakeup_reason();

    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) +
  " Seconds");

    co2Serial.begin(9600);
    pms.init();
    pinMode(9, INPUT);
    pinMode(15, INPUT);

    // Connect to Wi-Fi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

    // CO2 Measurement
    byte co2Cmd[9] = {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79};
    byte co2Response[9];

    co2Serial.write(co2Cmd, 9); // Request PPM CO2

    memset(co2Response, 0, 9);
    int i = 0;
    while (co2Serial.available() == 0) {
        delay(1000);
        i++;
    }
    if (co2Serial.available() > 0) {
        co2Serial.readBytes(co2Response, 9);
    }

    byte co2Check = getCheckSum(co2Response);
    if (co2Response[8] != co2Check) {
        Serial.println("Checksum not OK!");
        Serial.print("Received: ");
        Serial.println(co2Response[8]);
        Serial.print("Should be: ");
        Serial.println(co2Check);
    }
    
    int ppm_uart = 256 * (int)co2Response[2] + co2Response[3];
    byte temp = co2Response[4] - 40;
    byte status = co2Response[5];

    pms.read();                   // read the PM sensor
    noX = analogRead(15); // read the noX on pin 15

    // Send data via HTTP POST request
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;

        String postData = "{\"ppm\":" + String(ppm_uart) + ",\"temp\":" + String(temp) + ",\"pm01\":" + String(pms.pm01) + ",\"pm25\":" + String(pms.pm25) + ",\"pm10\":" + String(pms.pm10)  + ",\"noX\":" + String(random(150, 156) / 100.0, 2) + ",\"device\":" + "1" + "}";

        http.begin("https://" + String(serverName));
        http.addHeader("Content-Type", "text/plain");
        
        int httpResponseCode = http.POST(postData);
        
        if (httpResponseCode > 0) {
            Serial.print("HTTP Response code: ");
            Serial.println(httpResponseCode);
            String response = http.getString();
            Serial.println(response);
        } else {
            Serial.print("Error code: ");
            Serial.println(httpResponseCode);
        }

        http.end();
    }

    Serial.flush(); 
    esp_deep_sleep_start();
    
}

void loop() {
  // this is never run
}

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

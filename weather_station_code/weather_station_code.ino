#include "AS5600.h"
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "ThingSpeak.h"
#include <WiFi.h>
#include "time.h"

#define SEALEVELPRESSURE_HPA (1013.25)

const char* ssid = "yourSSID";   // your network SSID (name) 
const char* password = "yourPassword";   // your network password

// Setup the time server
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -18000;
const int   daylightOffset_sec = 3600;

AS5600 as5600;   // use default Wire
Adafruit_BME280 bme;

WiFiClient client;

uint8_t counter = 0;
const int sensorPin = 4; // GPIO pin where the hall effect sensor OUT is connected
const int rainPin = 13;
const int oneWireBus = 25; // GPIO where the temp sensor is connected
volatile int pulseCount = 0;
volatile int rainCount = 0;
volatile int wind_counter = 0;
volatile int rain_counter = 0;
volatile int sendCounter = 0;
float waterVolume;
float speedKnots;
float speedKPH;
int prevHour;
int prevDay;
int hour;
int day;
float waterCurrentHour = 0;
float waterCurrentDay = 0;
float waterTemperatureF;
float waterTemperatureC;
float temperatureC;
float humidity;
float pressureHPA;
float windDirection;

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

// API keys for ThingSpeak
unsigned long myChannelNumber = 1;
const char * myWriteAPIKey = "your_api_key";

unsigned long lastTime = 0;
unsigned long timerDelay = 30000;

struct tm timeinfo;

void IRAM_ATTR handlePulse() {
  pulseCount++;
}

void IRAM_ATTR handleRainPulse() {
  rainCount++;
}

void getDayAndHour(int &day, int &hour) {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    day = -1; // Return -1 in case of failure
    hour = -1;
    return;
  }

  day = timeinfo.tm_mday;  // Get the day of the month
  hour = timeinfo.tm_hour; // Get the hour of the day
}

void setup() {
  // Put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println(__FILE__);
  Serial.print("AS5600_LIB_VERSION: ");
  Serial.println(AS5600_LIB_VERSION);

  Wire.begin();

  as5600.begin(); // set software direction control. default parameter == 255
  as5600.setDirection(AS5600_CLOCK_WISE); // default, just be explicit.

  // Initialize the BME280
  if (!bme.begin(0x76)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }

  // Initialize the first hall effect sensor
  pinMode(sensorPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(sensorPin), handlePulse, RISING);
  
  pinMode(rainPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(rainPin), handleRainPulse, RISING);
  
  // Start the DS18B20 sensor
  sensors.begin();

  // Initialize the ESP32 as a WiFi station
  WiFi.mode(WIFI_STA);

  ThingSpeak.begin(client); // Initialize ThingSpeak

  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect");
    while (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(ssid, password); 
      delay(5000);     
    } 
    Serial.println("\nConnected.");
  }
  // Initialize and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  delay(2000);  // Wait for time to be synchronized

  // Initialize time-related variables
  getDayAndHour(day, hour);
  Serial.print("Day of the month: ");
  Serial.println(day);
  Serial.print("Hour of the day: ");
  Serial.println(hour);
}

void loop() {
  // Connect or reconnect to WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect");
    while (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(ssid, password); 
      delay(5000);     
    } 
    Serial.println("\nConnected.");
  }
  
  // Get angle information
  Serial.print(millis());
  Serial.print("\t");
  Serial.print(as5600.getDirection());
  Serial.print("\t");
  Serial.print(as5600.readAngle());
  Serial.print("\t");
  Serial.println(as5600.rawAngle() * AS5600_RAW_TO_DEGREES);

  windDirection = ((as5600.rawAngle() * AS5600_RAW_TO_DEGREES) - 24);
  if (windDirection < 0) {
    windDirection = windDirection + 360;
  }
  // Get temperature, pressure and humidity info
  
  temperatureC = bme.readTemperature();

  pressureHPA = bme.readPressure() / 100.0F;

  humidity = bme.readHumidity();

  if (wind_counter == 15) {
    //Serial.print("RPM: ");
    //Serial.println(pulseCount);
    speedKnots = pulseCount * 0.2769;
    speedKPH = pulseCount * 0.5128;
    //Serial.print("Speed in knots is: ");
    //Serial.println(speedKnots);
    //Serial.print("Speed in km/h is: ");
    //Serial.println(speedKPH);
    pulseCount = 0;
    wind_counter = 0;
  }
  wind_counter++;

  if (rain_counter == 15) {
    Serial.println(rainCount);
    waterVolume = rainCount * 0.4869 * 2;
    Serial.print("The following volume of water has fallen in the past 15 seconds: ");
    Serial.println(waterVolume);
    rainCount = 0;
    rain_counter = 0;

    
    
    getDayAndHour(day, hour);
    

    if (prevHour == hour) {
      waterCurrentHour += waterVolume;
    } else {
      waterCurrentHour = 0; 
    }
    prevHour = hour;

    if (prevDay == day) {
      waterCurrentDay += waterVolume;
    } else {
      waterCurrentDay = 0;
    }
    prevDay = day;
  }
  rain_counter++;

  // Get water temperature
  sensors.requestTemperatures(); 
  waterTemperatureC = sensors.getTempCByIndex(0);
  waterTemperatureF = sensors.getTempFByIndex(0);
  Serial.print(waterTemperatureC);
  Serial.println("ºC");
  Serial.print(waterTemperatureF);
  Serial.println("ºF");

  if (sendCounter == 15) {
    ThingSpeak.setField(1, temperatureC);
    ThingSpeak.setField(2, humidity);
    ThingSpeak.setField(3, pressureHPA);
    ThingSpeak.setField(4, waterCurrentHour);
    ThingSpeak.setField(5, waterCurrentDay);
    ThingSpeak.setField(6, waterTemperatureC);
    ThingSpeak.setField(7, speedKPH);
    ThingSpeak.setField(8, windDirection);
    
    int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

    if (x == 200) {
      Serial.println("Channel update successful.");
    } else {
      Serial.println("Problem updating channel. HTTP error code " + String(x));
    }
    sendCounter = 0;
  }
  sendCounter++;

  delay(1000);
}

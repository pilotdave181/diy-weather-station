# Weather Station with ESP32 + ThingSpeak Dashboard

## Overview

This weather station uses an **ESP32** microcontroller along with various sensors to monitor and report real-time environmental data, including temperature, humidity, pressure, wind speed, wind direction, rainfall, and water temperature. The data is uploaded to **ThingSpeak** for remote monitoring and visualization.
![PXL_20240827_204913090](https://github.com/user-attachments/assets/424ca3f5-f668-4f34-9d79-890946d32695)
![PXL_20240827_204901718 MP](https://github.com/user-attachments/assets/87eb6a1e-ef6c-4dda-a3b9-6a0560f68855)

## Features

- **Wind Speed**: Measures wind speed using a Hall effect sensor.
- **Wind Direction**: Determines wind direction with an AS5600 rotary encoder.
- **Temperature and Humidity**: Uses a **BME280** sensor for temperature, humidity, and pressure.
- **Rainfall**: Measures rainfall using a rain gauge.
- **Water Temperature**: Monitors water temperature with a **DS18B20** sensor.
- **Data Upload**: Uploads real-time data to **ThingSpeak** for remote monitoring.
  
## Components

- **ESP32** (Microcontroller)
- **AS5600** (Rotary Encoder for wind direction)
- **BME280** (Temperature, humidity, and pressure sensor)
- **Dallas DS18B20** (Water temperature sensor)
- **Hall Effect Sensor** (Wind speed)
- **Rain Gauge** (Rainfall measurement)

## Setup

1. **Wi-Fi**: Update the `ssid` and `password` variables in the code with your Wi-Fi credentials.
2. **ThingSpeak API Key**: Replace `myWriteAPIKey` with your **ThingSpeak API key**.
3. **Connect Sensors**: Connect all the sensors to the ESP32 as per the wiring in the code.
4. **NTP Time Server**: The code uses an NTP server to get accurate time.

## Installation

1. Download and install the required libraries:
   - **Adafruit BME280**
   - **DallasTemperature**
   - **ThingSpeak**
   - **AS5600**
2. Upload the code to your **ESP32**.
3. Connect your ESP32 to the Wi-Fi network.
4. Data will be sent to ThingSpeak every minute.

## Code Overview

- The **AS5600** measures the wind direction and converts raw angle data to degrees.
- The **BME280** sensor is used to read temperature, humidity, and atmospheric pressure.
- The **DallasTemperature** sensor reads water temperature.
- The **Hall Effect Sensor** counts pulses to determine wind speed.
- Rainfall is calculated based on pulse counts from the rain gauge.
- The system updates the ThingSpeak channel every minute with the collected data, including:
  - Temperature
  - Humidity
  - Pressure
  - Rainfall (hourly and daily)
  - Wind speed (in km/h)
  - Wind direction (in degrees)

## Usage

After uploading the code and connecting to the Wi-Fi, the data will start streaming to **ThingSpeak**. You can monitor the weather data in real-time via your **ThingSpeak dashboard**.

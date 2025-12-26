# Tiny BME280 Library #
[![Build Status](https://travis-ci.org/jasonacox/Tiny_BME280_Library.svg?branch=master)](https://travis-ci.org/jasonacox/Tiny_BME280_Library)

This is a Arduino library for the BME280 Humidity, Barometric Pressure + Temp sensor

## Description
This is a Arduino library for BME280 sensor for gathering temperature, humidity and pressure using I2C communication.  This library is a minimized fork of the more robust [Adafruit_BME280_Library](https://github.com/adafruit/Adafruit_BME280_Library) but with SPI and other features removed to reduce PROGMEM space.  This library is ideal for smaller controllers like the ATtiny85.

This library was designed to work with the Adafruit BME280 sensor in I2C mode.
 * http://www.adafruit.com/products/2652

## Setup
These sensors use I2C to communicate, using the I2C Logic pins:
* SCK - this is also the I2C clock pin, connect to the microcontrollers I2C clock line. On the ATtiny85 this is PB2 (physical pin 7)
* SDI or SDA - this is also the I2C data pin, connect to the microcontrollers I2C data line. On the ATtiny85 this is PB0 (physical pin 5)

Use of this library also requires the [Adafruit_Sensor](https://github.com/adafruit/Adafruit_Sensor) library to be installed on your system.

Download or clone this Tiny_BME280 library repo into your local Arduino libraries fold (e.g. `~/Documents/Arduino/libraries/` for MacOS) and restart the IDE.
```bash
cd ~/Documents/Arduino/libraries
git clone https://github.com/jasonacox/Tiny_BME280_Library.git 
```
This has been tested to work with ATtiny85 microcontrollers using the [ATTinyCore](https://github.com/SpenceKonde/ATTinyCore) core.

## Example Usage
```cpp
#include <Adafruit_Sensor.h>    // https://github.com/adafruit/Adafruit_Sensor
#include <Tiny_BME280.h>        // https://github.com/jasonacox/Tiny_BME280_Library

// Initialize BME280 Class
Tiny_BME280 bme; 

// Setup
void setup() {
    Serial.begin(9600);
    while(!bme.begin()) {
        Serial.println("BME280 not responding... waiting.");
        delay(1000);
    }
    Serial.println("BME280 activated");
}

// Loop
void loop() { 
    int temperature, pressure, humidity;

    temperature = bme.readTemperature();
    pressure = bme.readPressure() / 100.0;
    humidity = bme.readHumidity();

    Serial.print("Temperature = ");
    Serial.print(temperature);
    Serial.print("'C [");
    Serial.print((temperature * 1.8) + 32);
    Serial.println("'F]");
    
    Serial.print("Relative Humidity = ");
    Serial.print(humidity);
    Serial.println("%");

    Serial.print("Pressure = ");
    Serial.print(pressure);
    Serial.println(" hPa");
    
    Serial.println();

    delay(1000);
}
```


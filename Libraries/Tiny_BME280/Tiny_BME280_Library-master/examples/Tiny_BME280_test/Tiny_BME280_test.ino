#include <Adafruit_Sensor.h>
#include <Tiny_BME280.h>

// Initialize BME280 Class
Tiny_BME280 bme; 

// Setup
void setup() {
    Serial.begin(9600);

    Serial.println(F("TinyBME280 Test"));

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
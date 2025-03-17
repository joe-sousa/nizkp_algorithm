#include <Wire.h>
#include <Adafruit_INA219.h>

Adafruit_INA219 ina219(0x40);  // Sets the sensor addressr

int pinMonitorGeneratedAES = 10; // Pin used to monitor ciphered message with AES
int pinMonitorSentAES = 11; // Pin used to monitor packet assembly and transmission via BLE
bool cipheredMessageGenerated = false;  // Control variable to ensure that the reading occurs only once
bool packageSent = false; // Control variable to ensure that the reading occurs only once

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("Initializing the INA219 sensor...");
  if (!ina219.begin()) {
    Serial.println("Failed to initialize the INA219 sensor. Check the connections..");
    while (1); 
  }

 // Initialize the monitoring pins as input
  pinMode(pinMonitorGeneratedAES, INPUT);
  pinMode(pinMonitorSentAES, INPUT);
  Serial.println("INA219 sensor successfully initialized!");
}

void loop() {
  // Check if the monitoring pin is HIGH (indicating that the AES algorithm is ciphering the message in the Nano device)
  if (digitalRead(pinMonitorGeneratedAES) == HIGH && !cipheredMessageGenerated) {
    // Mark that the AES has been ciphered the message
    cipheredMessageGenerated = true;

    // Perform current, voltage, and power readings only when the pin is HIGH and the the message has been ciphered
    float current_mA = ina219.getCurrent_mA(); // Gets the current in mA
    float voltage_V = ina219.getBusVoltage_V(); // Gets the voltage in V
    float power_mW = current_mA * voltage_V; // Calculates the power in mW


    Serial.print("Current (mA) during AES ciphered message generation: ");
    Serial.println(current_mA);
    Serial.print("Voltage (V): ");
    Serial.println(voltage_V);
    Serial.print("Power (mW): ");
    Serial.println(power_mW);
    Serial.println("-----------------------");

  }else if (digitalRead(pinMonitorSentAES) == HIGH && !packageSent) {
    // Mark that the ciphered message and the message have been sent
    packageSent = true;

    // Perform current, voltage, and power readings only when the pin is HIGH and the package with the ciphered message and the message have been sent
    float current_mA = ina219.getCurrent_mA(); // Gets the current in mA
    float voltage_V = ina219.getBusVoltage_V();  // Gets the voltage in V
    float power_mW = current_mA * voltage_V; // Calculates the power in mW

    Serial.print("Current (mA) during the package sending: ");
    Serial.println(current_mA);
    Serial.print("Voltage (V): ");
    Serial.println(voltage_V);
    Serial.print("Power (mW): ");
    Serial.println(power_mW);
    Serial.println("-----------------------");

  }
}

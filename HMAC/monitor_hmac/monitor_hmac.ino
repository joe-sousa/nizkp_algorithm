#include <Wire.h>
#include <Adafruit_INA219.h>

Adafruit_INA219 ina219(0x40);  // Sets the sensor addressr

int pinMonitorGeneratedHmac = 10; // Pin used to monitor key generation
int pinMonitorSentHmac = 11; // Pin used to monitor packet assembly and transmission
bool hmacGenerated = false;  // Control variable to ensure that the reading occurs only once
bool hmacSent = false;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("Initializing the INA219 sensor...");
  if (!ina219.begin()) {
    Serial.println("Failed to initialize the INA219 sensor. Check the connections..");
    while (1); 
  }

 // Initialize the monitoring pins as input
  pinMode(pinMonitorGeneratedHmac, INPUT);
  pinMode(pinMonitorSentHmac, INPUT);
  Serial.println("INA219 sensor successfully initialized!");
}

void loop() {
  // Check if the monitoring pin is HIGH (indicating that the Nano is generating keys)
  if (digitalRead(pinMonitorGeneratedHmac) == HIGH && !hmacGenerated) {
    // Mark that the HMAC has been generated
    hmacGenerated = true;

    // Perform current, voltage, and power readings only when the pin is HIGH and the hmac has been generated
    float current_mA = ina219.getCurrent_mA(); // Gets the current in mA
    float voltage_V = ina219.getBusVoltage_V(); // Gets the voltage in V
    float power_mW = current_mA * voltage_V; // Calculates the power in mW


    Serial.print("Current (mA) during HMAC generation: ");
    Serial.println(current_mA);
    Serial.print("Voltage (V): ");
    Serial.println(voltage_V);
    Serial.print("Power (mW): ");
    Serial.println(power_mW);
    Serial.println("-----------------------");

  }else if (digitalRead(pinMonitorSentHmac) == HIGH && !hmacSent) {
    // Mark that the HMAC and the message have been sent
    hmacSent = true;

    // Perform current, voltage, and power readings only when the pin is HIGH and the HMAC and the message have been sent
    float current_mA = ina219.getCurrent_mA(); // Gets the current in mA
    float voltage_V = ina219.getBusVoltage_V();  // Gets the voltage in V
    float power_mW = current_mA * voltage_V; // Calculates the power in mW

    Serial.print("Current (mA) during HMAC and message sending: ");
    Serial.println(current_mA);
    Serial.print("Voltage (V): ");
    Serial.println(voltage_V);
    Serial.print("Power (mW): ");
    Serial.println(power_mW);
    Serial.println("-----------------------");

  }

  //delay(7000); // Waits for 2 seconds before the next check

}

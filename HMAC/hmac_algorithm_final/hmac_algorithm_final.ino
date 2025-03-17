#include <EEPROM.h>
#include <Crypto.h>       
#include <SHA256.h>       
#define KEY_BITS 256 // Size of the key to be applied in bits
#define KEY_BYTES (KEY_BITS / 8) // Size of the key in bytes
#define EEPROM_HASH_ADDR 64 // Address to save the hash
#define EEPROM_KEY_ADDR 2 // Address to save the encryption key
#define EEPROM_MSG_ADDR 323 // Address to save the message

#include <SoftwareSerial.h>

SoftwareSerial bluetooth(6, 7); // RX, TX for Arduino Nano communication with BLE module

byte key[KEY_BYTES];

// Function to generate a 256-bit random key 
void generateRandomKey(byte* key, int length) { 
  for (int i = 0; i < length; i++) { 
    key[i] = random(0, 256); // Generate a random byte 
  }
}

// Function to save or load the key from EEPROM
bool saveOrLoadKey(byte* key, int length, int startAddress) {
    // Check if the key already exists in EEPROM
    bool keyExists = true;
    for (int i = 0; i < length; i++) {
        if (EEPROM.read(startAddress + i) == 0xFF) {
            keyExists = false;
            break;
        }
    }

    if (keyExists) {
        // Load the key from EEPROM
        for (int i = 0; i < length; i++) {
            key[i] = EEPROM.read(startAddress + i);
        }
    } else {
        // Generate a new key and save it to EEPROM
        generateRandomKey(key, length);
        for (int i = 0; i < length; i++) {
            EEPROM.write(startAddress + i, key[i]);
        }
    }

    return keyExists;
}

// Function to build and send a data packet including HMAC and message
void build_pac() {

    uint8_t pac[48]; // Data packet (48 bytes) composed of HMAC and message
    byte buffer[48]; // Buffer to receive values from EEPROM and pass them to pac, including HMAC and message

    unsigned long start;
    unsigned long stop;
    float total;

    char clear_message[] = "TransactionDone!"; // Simulated payment confirmation message

    EEPROM.put(EEPROM_MSG_ADDR, clear_message); // Save the simulated payment confirmation message to EEPROM
    
    digitalWrite(3, HIGH); // Signal start of packet assembly and transmission to the energy monitor

    Serial.print(F("Building data package..."));
    Serial.println();
    start = micros();

    // Retrieve HMAC from EEPROM and add to pac
    EEPROM.get(EEPROM_HASH_ADDR, buffer);
    for (uint8_t i = 0; i < 32; i++)
      pac[i] = buffer[i];

    // Retrieve message from EEPROM and add it after the HMAC in pac
    EEPROM.get(EEPROM_MSG_ADDR, buffer);
    for (uint8_t i = 0; i < 16; i++){
      pac[i + 32] = buffer[i];
    }

    // Send the data packet including HMAC, message, and "PA" to confirm the transmission
    send(pac, 48, F("PA"));
    
    // End signal for packet assembly and transmission
    digitalWrite(3, LOW);

    stop = micros();
    total = (stop - start)/1000000.0;
    Serial.print("Time to build and send the packet: ");
    Serial.print(total, 3);
    Serial.print(" seconds");
    bluetooth.print(total);
}

// Function to print the generated key in hexadecimal format
void printKeyInHex(const byte* key, int length) {
  Serial.print("Generated Key: "); 
  for (int i = 0; i < length; i++) { 
    if (key[i] < 16)
      Serial.print('0');
    Serial.print(key[i], HEX);
  }
  Serial.println(); 
}

// Function to save the key to EEPROM
void saveKeyToEEPROM(const byte* key, int length, int startAddress) { 
  EEPROM.put(startAddress, key); 
}

// Function to load the key from EEPROM
void loadKeyFromEEPROM(byte* key, int length, int startAddress) {
  EEPROM.get(startAddress, key); 
}

// Function to save the HMAC (hash digest) to EEPROM
void saveHashToEEPROM(const byte* hash, int length, int addr) {
  for (int i = 0; i < length; i++) {
    EEPROM.write(addr + i, hash[i]);
  }
}

// Function to load the HMAC (hash digest) from EEPROM
void loadHashFromEEPROM(byte* hash, int length, int addr) {
  for (int i = 0; i < length; i++) {
    hash[i] = EEPROM.read(addr + i);
  }
}

// Function to convert byte values to hexadecimal and prepare them for BLE transmission
void send(uint8_t *seq, uint8_t len, String operation) {
    const char hexchars[] = "0123456789ABCDEF"; 
    String hexString = "";
    bluetooth.print(operation); 
    for (uint8_t posn = 0; posn < len; ++posn) { 
        // Build the hexadecimal string
        hexString += hexchars[(seq[posn] >> 4) & 0x0F];
        hexString += hexchars[seq[posn] & 0x0F];
        
        bluetooth.print(hexchars[(seq[posn] >> 4) & 0x0F]); 
        bluetooth.print(hexchars[seq[posn] & 0x0F]); 
    } 
    bluetooth.println(';'); 
    Serial.print("Sending: "); 
    Serial.print(operation); 
    Serial.println(';'); 

    // Print the full hexadecimal string
    Serial.print("Hexadecimal String (hmac + message) Sent: ");
    Serial.println(hexString);
}

// Function to initialize and load the key
void initializeKey(byte* key, int keyLength, int keyBits, int eepromAddr) {
  // Inicializar a função random
  randomSeed(analogRead(0));

  // Save or load the key
  saveOrLoadKey(key, keyLength, eepromAddr);

  // Load the key from EEPROM
  loadKeyFromEEPROM(key, keyBits, eepromAddr);

  // Print the loaded/generated key
  printKeyInHex(key, keyLength);
  Serial.println();
}

// Function to generate HMAC
void generateHMAC(const byte* key, int keyLength, const char* msg, byte* hmacResult, int hmacLength) {
  // Initialize the SHA256 object
  SHA256 sha256;
  sha256.resetHMAC(key, keyLength);
  
  // Add the message to the HMAC
  sha256.update((const uint8_t*)msg, strlen(msg));

  // Finalize the HMAC
  sha256.finalizeHMAC(key, keyLength, hmacResult, hmacLength);
}

void setup() {
  Serial.begin(9600);
  bluetooth.begin(9600);
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  delay(2000);

  // Initialize and load the key
  initializeKey(key, KEY_BYTES, KEY_BITS, EEPROM_KEY_ADDR);  

  // Buffer for the message
  char msg[17];

  // Retrieve the message from EEPROM and save it to the variable msg
  EEPROM.get(EEPROM_MSG_ADDR, msg);

  // Set pin 2 to HIGH to start measuring energy consumption for HMAC generation
  digitalWrite(2, HIGH);

  // Mark the start time to measure the duration for HMAC generation 
  unsigned long start = micros();

  // Buffer for the HMAC
  byte hmacResult[32];

  Serial.println(msg);
  // Generate the HMAC
  generateHMAC(key, KEY_BYTES, msg, hmacResult, sizeof(hmacResult));

  // Set pin 2 to LOW to stop measuring energy consumption for HMAC generation
  digitalWrite(2, LOW);
  
  // Mark the end time to measure the duration for HMAC generation
  unsigned long end = micros();

  // Calculate the execution time for generating the HMAC
  float execution_time = (end - start)/100000.0;

  // Verify the message and print as string
  Serial.print("Retrieved Message: ");
  Serial.println(msg);

  // Verify the generated HMAC before saving
  Serial.print("Generated HMAC: ");
  for (int i = 0; i < 32; i++) {
    Serial.print(hmacResult[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  // Print the execution time
  Serial.print("Time to generate HMAC: ");
  Serial.print(execution_time, 3);
  Serial.println(" seconds");

  bluetooth.print(execution_time);

  // Save the hash to EEPROM
  saveHashToEEPROM(hmacResult, 32, EEPROM_HASH_ADDR);
  
  
  // Build and send the packet
  build_pac();
  
}

void loop() {

}

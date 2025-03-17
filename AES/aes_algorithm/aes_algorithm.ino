#include <EEPROM.h>
#include <Crypto.h>       
#include <AESLib.h>       
#define KEY_BITS 256 // Size of the key to be applied in bits
#define KEY_BYTES (KEY_BITS / 8) // Size of the key in bytes
#define EEPROM_CIPHERED_TEXT_ADDR 66 // Address to save the hash
#define EEPROM_KEY_ADDR 1 // Address to save the encryption key
#define EEPROM_MSG_ADDR 335 // Address to save the message

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

// Function to build and send a data packet including ciphered text and message
void build_pac() {

    uint8_t pac[32]; // Data packet (32 bytes) composed of ciphered text and message
    byte buffer[32]; // Buffer to receive values from EEPROM and pass them to pac, including ciphered text and message

    unsigned long start;
    unsigned long stop;
    float total;

    digitalWrite(3, HIGH); // Signal start of packet assembly and transmission to the energy monitor

    Serial.print(F("Building data package..."));
    Serial.println();
    start = micros();

    // Retrieve ciphered text with 16 bytes from EEPROM and add to pac
    EEPROM.get(EEPROM_CIPHERED_TEXT_ADDR, buffer);
    for (uint8_t i = 0; i < 16; i++)
      pac[i] = buffer[i];

    // Retrieve message from EEPROM and append it to the end of the ciphered text in pac
    EEPROM.get(EEPROM_MSG_ADDR, buffer);
    for (uint8_t i = 0; i < 16; i++){
      pac[i + 16] = buffer[i];
    }

    // Send the data packet including the ciphered message, message, and "PA" to confirm the transmission
    send(pac, 32, F("PA"));
    
    // End signal for packet assembly and transmission
    digitalWrite(3, LOW);

    stop = micros();
    total = (stop - start)/1000000.0;
    Serial.print("Time to build and send the packet: ");
    Serial.print(total, 5);
    Serial.print(" seconds");

    char buffer1[10];  // Buffer para armazenar o número formatado
    dtostrf(total, 6, 6, buffer1);  // (valor, largura mínima, casas decimais, buffer)
    bluetooth.print(buffer1);
    //bluetooth.print(total);
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

// Function to save the ciphered text to EEPROM
void saveCipheredTextToEEPROM(const byte* cipheredText, int length, int addr) {
  for (int i = 0; i < length; i++) {
    EEPROM.write(addr + i, cipheredText[i]);
  }
}

// Function to load the ciphered text from EEPROM
void loadCipheredTextToEEPROM(byte* cipheredText, int length, int addr) {
  for (int i = 0; i < length; i++) {
    cipheredText[i] = EEPROM.read(addr + i);
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
    Serial.print("Hexadecimal String (Ciphered Text + message) Sent: ");
    Serial.println(hexString);
}

// Function to initialize and load the key
void initializeKey(byte* key, int keyLength, int keyBits, int eepromAddr) {
  // Initialize random function using an analog read value
  randomSeed(analogRead(0));

  // Save or load the key
  saveOrLoadKey(key, keyLength, eepromAddr);

  // Load the key from EEPROM
  loadKeyFromEEPROM(key, keyBits, eepromAddr);

  // Print the loaded/generated key
  printKeyInHex(key, keyLength);
  Serial.println();
}

// Function to cipher the original message
void cipherText(const byte* key, char* msg, byte* cipherText) {
  //Copy the original message from msg to cipherText
  memcpy(cipherText, msg, 16);
  //Call the funtion aes256_enc_single() passing the key and  the message to generate the ciphered text
  aes256_enc_single(key, cipherText);
  // Print the ciphered text in hexadecimal
  Serial.print("Ciphered text: ");
  for (int i = 0; i < 16; i++) {
    if ((uint8_t)cipherText[i] < 16) {
      Serial.print('0');  // Adicionar zero à esquerda para valores menores que 16
    }
    Serial.print((uint8_t)cipherText[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
  
}


void setup() {
  Serial.begin(9600);
  bluetooth.begin(9600);
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  delay(2000);

  // Set pin 2 to HIGH to measure energy consumption during ciphered message generation
  digitalWrite(2, HIGH);

  // Mark the start time to measure the duration for ciphered message generation
  unsigned long start = micros();

  // Initialize and load the key
  initializeKey(key, KEY_BYTES, KEY_BITS, EEPROM_KEY_ADDR);

  char clear_message[] = "PaymentConfirmed"; // Simulated payment confirmation message with 16 bytes

  EEPROM.put(EEPROM_MSG_ADDR, clear_message); // Save the simulated payment confirmation message to EEPROM

  // Buffer for the message
  char msg[17];

  // Retrieve the message from EEPROM and save it to the variable msg
  EEPROM.get(EEPROM_MSG_ADDR, msg);

  byte ciphered_message[16]; // Buffer to store the message wich must be ciphered

  // Cipher the message before storing it in EEPROM
  cipherText(key, msg, ciphered_message);

  //Store the ciphered message in EEPROM at address
  EEPROM.put(EEPROM_CIPHERED_TEXT_ADDR, ciphered_message);

  // Set pin 2 to LOW to stop measuring energy consumption for ciphered message generation
  digitalWrite(2, LOW);

  // Mark the end time to measure the ciphered message generation
  unsigned long end = micros();

  // Verify the message and print as string
  Serial.print("Retrieved Message: ");
  Serial.println(msg);
  
  Serial.println();

  // Calculate the execution time for ciphered message generation
  float execution_time = (end - start)/1000000.0;

  // Print the execution time
  Serial.print("Time to generate AES: ");
  Serial.print(execution_time, 5);
  Serial.println(" seconds");

  char buffer[10];  // Tamanho suficiente para armazenar o valor convertido
  dtostrf(execution_time, 6, 6, buffer);  // (valor, largura mínima, casas decimais, buffer)
  bluetooth.print(buffer);
  //bluetooth.print(execution_time);
  
  // Build and send the packet
  build_pac();
}

void loop() {

}

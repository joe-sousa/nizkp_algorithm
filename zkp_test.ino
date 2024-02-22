#include <Arduino.h>
#include <uECC.h>
#include <uECC_vli.h>
#include <types.h>
#include <EEPROM.h>
#include <SoftwareSerial.h>
#include <AESLib.h>
#include <SHA256.h>

SoftwareSerial bluetooth(10, 11); // RX, TX

int memoryTest()
{
    int byteCounter = 0; // initialize a counter
    byte *byteArray;     // create a pointer to a byte array
    // More on pointers here: http://en.wikipedia.org/wiki/Pointer#C_pointers

    // use the malloc function to repeatedly attempt
    // allocating a certain number of bytes to memory
    // More on malloc here: http://en.wikipedia.org/wiki/Malloc
    while ((byteArray = (byte *)malloc(byteCounter * sizeof(byte))) != NULL)
    {
        byteCounter++;   // if allocation was successful, then up the count for the next try
        free(byteArray); // free memory after allocating it
    }

    free(byteArray);    // also free memory after the function finishes

    return byteCounter; // send back the highest number of bytes successfully allocated
}
//*****************************************************************************************************



static int RNG(uint8_t *dest, unsigned size)
{
    // Use the least-significant bits from the ADC for an unconnected pin (or connected to a source of
    // random noise). This can take a long time to generate random data if the result of analogRead(0)
    // doesn't change very frequently.
    while (size)
    {
        uint8_t val = 0;
        for (unsigned i = 0; i < 8; ++i)
        {
            int init = analogRead(0);
            int count = 0;
            while (analogRead(0) == init)
                ++count;

            if (count == 0)
                val = (val << 1) | (init & 0x01);
            else
                val = (val << 1) | (count & 0x01);
        }
        *dest = val;
        ++dest;
        --size;
    }
    // NOTE: it would be a good idea to hash the resulting random data using SHA-256 or similar.
    return 1;
}

void print_hex(uint8_t seq[], uint8_t len)
{
    static const char hexchars[] = "0123456789ABCDEF";
    for (uint8_t posn = 0; posn < len; ++posn)
    {
        Serial.print(hexchars[(seq[posn] >> 4) & 0x0F]);
        Serial.print(hexchars[seq[posn] & 0x0F]);
    }
    Serial.println();
}

String decToHex(uint8_t data[], uint8_t len)
{
    String valeuHex = "";
    static const char hexchars[] = "0123456789ABCDEF";
    for (uint8_t posn = 0; posn < len; ++posn)
    {
        String hex = " ";
        hex = (hexchars[(data[posn] >> 4) & 0x0F]);
        hex = hex + (hexchars[data[posn] & 0x0F]);
        valeuHex += hex;
    }
    return valeuHex;
}

void send(String operation)
{
    bluetooth.print(operation);
    bluetooth.println(';');
}

void send(uint8_t *seq, uint8_t len, String operation)
{
    const char hexchars[] = "0123456789ABCDEF";
    bluetooth.print(operation);
    for (uint8_t posn = 0; posn < len; ++posn)
    {
        bluetooth.print(hexchars[(seq[posn] >> 4) & 0x0F]);
        bluetooth.print(hexchars[seq[posn] & 0x0F]);
        if (posn % 18 == 0)
            delay(500);
    }
    bluetooth.println(';');
}

void reverse(uECC_word_t *array, uint8_t len)
{
    int8_t aux;
    for (uint8_t i = 0; i < len / 2; i++)
    {
        aux = array[i];
        array[i] = array[len - i - 1];
        array[len - i - 1] = aux;
    }
}

void generate_shared_secret()
{
    String secret_hex;
    char secret_char[32];
    uint8_t secret_byte[32];
    uint8_t pub_key_adm[64];
    uint8_t pri_key[32];
    uint8_t shared_secret[32];
    uECC_Curve curve = uECC_secp256k1();

    EEPROM.get(1, pri_key);
    EEPROM.get(97, pub_key_adm);

    Serial.print(F("Calculating shared secret... "));
    unsigned long start = millis();

        uECC_shared_secret(pub_key_adm, pri_key, shared_secret, curve);

        secret_hex = decToHex(shared_secret, 32);
        secret_hex.toCharArray(secret_char, 33);

        for (uint8_t i = 0; i < 32; i++)
            secret_byte[i] = (uint8_t) secret_char[i]; 

    unsigned long stop = millis();
    float total = (stop - start) / 1000.00;
    Serial.println(total, 3);

    EEPROM.put(161, secret_byte);
}

void generateKeyPair()
{
    uint8_t pri_key[32];
    uint8_t pub_key[64];
    uECC_Curve curve = uECC_secp256k1();

    Serial.print(F("Generating the public/private key pair... "));
    unsigned long start = millis();
        uECC_make_key(pub_key, pri_key, curve);
    unsigned long stop = millis();
    float total = (stop - start) / 1000.00;
    Serial.println(total, 3);
   
    EEPROM.put(1, pri_key);
    EEPROM.put(33, pub_key);

    
}

void processAdmPublicKey(String data)
{
    uint8_t dec;
    char hex_key[129];
    uint8_t pub_key_adm[64];

    data.toCharArray(hex_key, 129);

    const String hexchars = "0123456789ABCDEF";
    for (uint8_t i = 0; i < 128; i += 2)
    {
        dec = hexchars.indexOf(hex_key[i]);
        dec = dec << 4;
        dec = dec + hexchars.indexOf(hex_key[i + 1]);
        pub_key_adm[i / 2] = dec;
    }

    //Record administrator's public key in the EEPROM memory
    EEPROM.put(97, pub_key_adm);
}

void initialAction()
{
    uint8_t reg;
    EEPROM.get(340, reg);

    if (reg == 0)
    {
        //Sending acceptance information
        send(F("RA"));
        generateKeyPair();
    }
    else
        send(F("R1")); //Device is already registered
}

void defineIdentification(String data)
{
    char id_char[3];
    uint8_t pub_key[64];
    uint8_t id;

    // Registering the beginning time
    unsigned long start = millis();

    //Registering the device's ID
    data.substring(0, 3).toCharArray(id_char, 3);
    EEPROM.put(194, atoi(id_char)); //Record ID in EEPROM memory
    EEPROM.get(194, id);

    //Retrieving and sending the device's public key
    EEPROM.get(33, pub_key);
    send(pub_key, 64, F("RK"));

    // Registering the end time
    unsigned long end = millis();

    // Calculating the execution time
    unsigned long execution_time = end - start;
    // Printing the execution time
    Serial.print(F("Execution time of the function defineIdentification(): "));
    Serial.print(execution_time);
    Serial.println(F(" milissegundos"));
}

void generate_shared_point()
{
    uint8_t point[64];
    uint8_t witness[32];
    uECC_Curve curve = uECC_secp256k1();

    //Generating a random number w (witness) and the point that will be sent to the verifier
    uECC_make_key(point, witness, curve);

    Serial.print(F("witness: "));
    print_hex(witness,32);

    EEPROM.put(195, point);
    EEPROM.put(291, witness);    
}

void calc_challenge(uECC_word_t *hash, const uECC_word_t *G, uint8_t *pub_key, uint8_t *commit)
{
    SHA256  hasher;
    uint8_t tmp[1];
    char    aux[2];
    char    msg[192];
    
    Serial.print(F("Calculating challenge... "));
    unsigned long start = millis();
    
    for (uint8_t i = 0; i < 32; i++){
        tmp[0] = G[31-i];
        decToHex(tmp, 1).toCharArray(aux, 3);
        msg[i*2] = aux[0];
        msg[i*2+1] = aux[1];
    }

    for (uint8_t i = 32; i < 64; i++){
        tmp[0] = pub_key[i % 32];
        decToHex(tmp, 1).toCharArray(aux, 3);
        msg[i*2] = aux[0];
        msg[i*2+1] = aux[1];
    }

    for (uint8_t i = 64; i < 96; i++){
        tmp[0] = commit[i % 64];
        decToHex(tmp, 1).toCharArray(aux, 3);
        msg[i*2] = aux[0];
        msg[i*2+1] = aux[1];
    }

    hasher.update(msg, 192);
    hasher.finalize(hash, 32);

    unsigned long stop = millis();
    float total = (stop - start) / 1000.00;
    Serial.println(total, 3);
    
    Serial.print(F("hash: "));
    print_hex(hash,32);
}

void calc_mult_mod(uECC_word_t *mult, uint8_t *hash, uint8_t *pri_key, const uECC_word_t *p)
{
    reverse(hash, 32);
    reverse(pri_key, 32);
    uECC_vli_modMult(mult, hash, pri_key, p, 32); //Calculate (hash · priK) mod p
}

void calc_add_mod(uECC_word_t *answer, uint8_t witness[], uECC_word_t mult[], const uECC_word_t *p)
{
    reverse(witness, 32);
    uECC_vli_modAdd(answer, witness, mult, p, 32);
    reverse(answer, 32);
}

void calculate_NIZKP()
{
    uint8_t pub_key[32];
    uint8_t point[64]; 
    uint8_t scalar[32];
    uint8_t answer[32];
    uECC_word_t cha[32];
    uECC_word_t multip[32];

    uECC_Curve curve = uECC_secp256k1();
    const uECC_word_t *G = uECC_curve_G(curve);
    const uECC_word_t *n = uECC_curve_n(curve);

    //Retrieving data from EEPROM memory
    EEPROM.get(1, scalar);
    EEPROM.get(33, pub_key);
    EEPROM.get(195, point);

    Serial.print(F("Chave privada: "));
    print_hex(scalar,32);

    //Calculating the challenge 'cha' and the response 'ans' that will be sent to the verifier
    calc_challenge(cha, G, pub_key, point); //Calculate hash (G||B||A)
    
    calc_mult_mod(multip, cha, scalar, n);
    EEPROM.get(291, scalar);
    calc_add_mod(answer, scalar, multip, n);

   
    Serial.print(F("resposta: "));
    print_hex(answer,32);
    
    EEPROM.put(259, answer); //Recording response to EEPROM
}

void build_pac()
{
    uint8_t pac[113];
    uint8_t buffer[64];
    uint8_t id;

    unsigned long start;
    unsigned long stop;
    float total;

    Serial.print(F("Building data package..."));
    start = millis();

    //Ponto compartilhado
    EEPROM.get(195, buffer);
    for (uint8_t i = 0; i < 64; i++)
        pac[i] = buffer[i];

    // Resposta ao desafio
    EEPROM.get(259, buffer);
    for (uint8_t i = 0; i < 32; i++)
        pac[i + 64] = buffer[i];

    //ID do dispositivo
    EEPROM.get(194, id);
    pac[96] = id;

    //Dados cifrados
    EEPROM.get(323, buffer);
    for (uint8_t i = 0; i < 16; i++)
        pac[i + 97] = buffer[i];

    send(pac, 113, F("PA"));

    stop = millis();
    total = (stop - start) / 1000.00;
    Serial.println(total, 3);
}

void generate_pacNIZKP()
{
    char data[] = "pressure:120x075"; //Static data just to represent health data.
    uint8_t secret[32];
    EEPROM.get(161, secret);
    unsigned long start;
    unsigned long stop;
    float total;

    Serial.print(F("Encrypting data... "));
    start = millis();
        aes256_enc_single(secret, data);
        //Gravando dados cifrados na memória EEPROM
        EEPROM.put(323, data);
    stop = millis();
    total = (stop - start) / 1000.00;
    Serial.println(total, 3);

    Serial.print(F("Generating NIZKP... "));
    start = millis();
        generate_shared_point();
        calculate_NIZKP();
    stop = millis();
    total = (stop - start) / 1000.00;
    Serial.println(total, 3);
}

unsigned long begin, start;
unsigned long end, stop;
float time, total;
   
void setup()
{
    /********************Memory Positions********************
    * Pos  Data
    * 0    Reset
    * 1    Private key (monitoring device)
    * 33   Public key  (monitoring device)
    * 97   Public key  (administrator device)
    * 161  Shared secret
    * 194  ID
    * 195  Committed point
    * 259  Answer
    * 291  Witness
    * 323  Encrypted data
    * 340  Registration
    ********************************************************/
    
    Serial.begin(115200);
    bluetooth.begin(9600);
    uECC_set_rng(&RNG);
    EEPROM.put(340, 0);
}

void loop()
{ 
    
    if (bluetooth.available())
    {
        char *data = (char *)malloc(130 * sizeof(char));
        bluetooth.readString().toCharArray(data, 130);

        switch (data[0])
        {
        case 'R': //Registration request
            begin = millis();
            initialAction();
            free(data);
            break;
        case 'I': //Establishment of identification
            start = millis(); //Start of parameter change
            defineIdentification(String(data).substring(1, 3));
            free(data);
            break;
        case 'K': //Receiving the public key from the administrator device
            processAdmPublicKey(String(data).substring(1,130));
            stop = millis(); //End of parameter change
            total = (stop - start) / 1000.00;
            Serial.print(F("Changing parameters: "));
            
            generate_shared_secret();
            EEPROM.put(340, 1);
            free(data);
            end = millis();
            time = (end - begin) / 1000.00;
            Serial.print(F("Total device registration time: "));
            break;
        case 'D': //Generation and transmission of NIZKP
            free(data);
            begin = millis();
                generate_pacNIZKP();
                build_pac();
            break;
        case 'E':
            free(data);
            end = millis();
            time = (end - begin) / 1000.00;
            Serial.print(F("Total NIZKP generation/verification time: "));
            Serial.println(time, 3);
            Serial.print(F("Free memory: "));
            Serial.println(memoryTest());
            Serial.print(F("End"));
        break;
        default:
            //Serial.println("Chegou");
            free(data);
            break;
        }
    }

}

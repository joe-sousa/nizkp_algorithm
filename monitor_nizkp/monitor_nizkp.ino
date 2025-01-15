#include <Wire.h>
#include <Adafruit_INA219.h>

Adafruit_INA219 ina219(0x40);  // Sets the sensor addressr

int pinoMonitoraParChaves = 10; // Pin used to monitor key generation
int pinoMonitoraEnvioPacote = 11; // Pin used to monitor packet assembly and transmission
bool chaveGerada = false;  // Control variable to ensure that the reading occurs only once
bool pacoteEnviado = false;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("Iniciando o sensor INA219...");
  if (!ina219.begin()) {
    Serial.println("Falha ao iniciar o sensor INA219. Verifique as conexões.");
    while (1);  // Pausa o programa se houver erro
  }

  // Inicializa oa pinos de monitoramentos como entrada
  pinMode(pinoMonitoraParChaves, INPUT);
  pinMode(pinoMonitoraEnvioPacote, INPUT);
  Serial.println("Sensor INA219 iniciado com sucesso!");
}

void loop() {
  // Verifica se o pino de monitoração está HIGH (indicando que o Nano está gerando chaves)
  if (digitalRead(pinoMonitoraParChaves) == HIGH && !chaveGerada) {
    // Marca que a chave foi gerada
    chaveGerada = true;

    // Realiza a leitura de corrente, tensão e potência apenas quando o pino está HIGH e a chave foi gerada
    float corrente_mA = ina219.getCurrent_mA(); // Obtém a corrente em mA
    float tensao_V = ina219.getBusVoltage_V();  // Obtém a tensão em V
    float potencia_mW = corrente_mA * tensao_V; // Calcula a potência em mW

    Serial.print("Corrente (mA) na geração do par de chaves: ");
    Serial.println(corrente_mA);
    Serial.print("Tensão (V): ");
    Serial.println(tensao_V);
    Serial.print("Potência (mW): ");
    Serial.println(potencia_mW);
    Serial.println("-----------------------");
  }else if (digitalRead(pinoMonitoraEnvioPacote) == HIGH && !pacoteEnviado) {
    // Marca que o pacote foi enviado
    pacoteEnviado = true;

    // Realiza a leitura de corrente, tensão e potência apenas quando o pino está HIGH e a chave foi gerada
    float corrente_mA = ina219.getCurrent_mA(); // Obtém a corrente em mA
    float tensao_V = ina219.getBusVoltage_V();  // Obtém a tensão em V
    float potencia_mW = corrente_mA * tensao_V; // Calcula a potência em mW

    Serial.print("Corrente (mA) na montagem e envio do pacote: ");
    Serial.println(corrente_mA);
    Serial.print("Tensão (V): ");
    Serial.println(tensao_V);
    Serial.print("Potência (mW): ");
    Serial.println(potencia_mW);
    Serial.println("-----------------------");
  }

  delay(2000); // Aguarda 2 segundos antes da próxima verificação
}

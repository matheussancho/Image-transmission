/*

///////////////////
// RECEPTOR
///////////////////

// Bibliotecas LoRa e display
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Definir pinos usados pelo módulo LoRa
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26

// Frequência
#define BAND 915E6

// Definir pinos do OLED
#define OLED_SDA 4
#define OLED_SCL 15
#define OLED_RST 16
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

const int packet_size = 240;
const int total_packets = 3500;  // Para a imagem RGB 528x528
uint16_t packet_index = 0;  // Agora "uint16_t" para suportar até 65535 pacotes
bool all_packets_received = true;
// Adicionar variáveis globais para o progresso e erros
uint16_t packet_error_count = 0;
uint16_t packet_received_count = 0;

// Inicializar display OLED
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

// Função para calcular checksum
uint8_t calculateChecksum(const uint8_t *data, size_t len) {
  uint8_t checksum = 0;
  for (size_t i = 0; i < len; i++) {
    checksum ^= data[i];
  }
  return checksum;
}

void setup() {
  // Inicializar monitor serial
  Serial.begin(115200);

  // Resetar o display OLED
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);

  // Inicializar OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C, false, false);
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Inicializando LoRa...");
  display.display();

  // Inicializar comunicação LoRa
  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DIO0);

  if (!LoRa.begin(BAND)) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Falha no LoRa");
    display.display();
    while (1);
  }

  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("LoRa Inicializado");
  display.display();
}

void loop() {
  int packetSize = LoRa.parsePacket();
  
  if (packetSize == (packet_size + 3)) {  // 240 bytes + 2 de sequência + 1 de checksum
    uint8_t packet[packet_size + 3];
    LoRa.readBytes(packet, packetSize);

    // Integrity
    uint16_t sequence = (packet[0] << 8) | packet[1];  // Atualizado para uint16_t
    uint8_t checksum_received = packet[packet_size + 2];
    uint8_t checksum_calculated = calculateChecksum(packet + 2, packet_size);

    if (checksum_received == checksum_calculated && sequence == packet_index) {
      packet_received_count++;  // Contador de pacotes recebidos com sucesso
      packet_index++;

      // Atualizar o display com o progresso e pacotes recebidos
      display.clearDisplay();
      display.setCursor(0, 0);
      display.print("Pacote ");
      display.print(packet_received_count);
      display.print("/");
      display.print(total_packets);
      display.setCursor(0, 10);
      display.print("Erros: ");
      display.print(packet_error_count);
      display.display();

    } else {
      packet_error_count++;  // Contador de pacotes com erro
      Serial.println("Erro: Checksum ou sequência incorreta.");

      display.clearDisplay();
      display.setCursor(0, 0);
      display.print("Erro no pacote ");
      display.print(sequence);
      display.setCursor(0, 10);
      display.print("Erros: ");
      display.print(packet_error_count);
      display.display();

      // Aqui você pode decidir se quer solicitar o reenvio ao sender via ACK.
    }

    if (packet_index >= total_packets) {
      display.clearDisplay();
      display.setCursor(0, 0);
      if (packet_error_count == 0) {
        display.print("Recebido com sucesso!");
      } else {
        display.print("Recebimento concluido!");
        display.setCursor(0, 10);
        display.print("Erros: ");
        display.print(packet_error_count);
      }
      display.display();
      Serial.println("Todos os pacotes recebidos.");
      while (1);  // Pausar após a recepção de todos os pacotes
    }
  }
}

*/

///////////////////
//    RECEIVER   //
///////////////////
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "lena_rgb.h"  // Imagem Lenna em bytes

// Definições dos pinos do LoRa
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26

// Frequência LoRa
#define BAND 915E6

// Pinos do OLED
#define OLED_SDA 4
#define OLED_SCL 15
#define OLED_RST 16
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

const int packet_size = 200;
const int total_packets = sizeof(lena_rgb) / packet_size + (sizeof(lena_rgb) % packet_size != 0);
uint16_t packet_index = 0;  // Agora é uint16_t
uint16_t error_count = 0;  // Contagem de erros

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

// Função para calcular o checksum
uint16_t calculateChecksum(const uint8_t *data, size_t len) {
  uint16_t checksum = 0;
  for (size_t i = 0; i < len; i++) {
    checksum ^= data[i];  // Operação XOR
  }
  return checksum;
}

// Função para enviar ACK ou NACK
void sendACK(bool success) {
  LoRa.beginPacket();
  LoRa.write(success ? 1 : 0);  // 1 para ACK, 0 para NACK
  LoRa.endPacket();
}

void setup() {
  Serial.begin(115200);

  // Inicializar OLED
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(100);
  digitalWrite(OLED_RST, HIGH);
  
  Wire.begin(OLED_SDA, OLED_SCL);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C, false, false);
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Inicializando LoRa...");
  display.display();
  delay(5000);

  // Inicializar LoRa
  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DIO0);
  if (!LoRa.begin(BAND)) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Falha no LoRa");
    display.display();
    while (1);
  }

  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("LoRa Inicializado");
  display.display();
}

/*

void loop() {
  int packetSize = LoRa.parsePacket();
  unsigned long start_time = millis();  // Marca o tempo de início da recepção

  // Verifica se o tamanho do pacote está correto
  while (millis() - start_time < 5000) {  // Espera até 5 segundos por pacotes
    packetSize = LoRa.parsePacket();
    if (packetSize == (packet_size + 3)) {  // Pacote: índice (2 bytes) + dados + checksum (1 byte)
      uint8_t packet[packet_size + 3];
      LoRa.readBytes(packet, packetSize);

      // Extrair os dois bytes do índice do pacote
      uint16_t sequence = packet[0] | (packet[1] << 8);  // Combina os dois bytes para formar o índice completo
      uint16_t checksum_received = packet[packet_size + 2];  // Último byte é o checksum
      uint16_t checksum_calculated = calculateChecksum(packet, packet_size + 2);  // Checksum do índice + dados

      Serial.print("Pacote recebido: ");
      Serial.println(sequence);
      Serial.print("Checksum recebido: ");
      Serial.println(checksum_received, HEX);
      Serial.print("Checksum calculado: ");
      Serial.println(checksum_calculated, HEX);

      // Verifica a integridade do pacote
      if (checksum_received == checksum_calculated && sequence == packet_index) {
        Serial.print("Pacote ");
        Serial.print(sequence);
        Serial.println(" recebido corretamente.");

        // Atualiza o display com o status do pacote recebido
        display.clearDisplay();
        display.setCursor(0, 0);
        display.print("Pacote ");
        display.print(sequence + 1);  // Mostra o próximo pacote esperado
        display.print("/");
        display.print(total_packets);
        display.display();

        // Envia ACK para confirmar o recebimento do pacote
        sendACK(true);
        packet_index++;  // Avança para o próximo pacote
      } else {
        Serial.print("Erro no pacote ");
        Serial.println(sequence);
        error_count++;

        // Exibe as informações de erro no display
        display.clearDisplay();
        display.setCursor(0, 0);
        display.print("Erro no pacote: ");
        display.print(sequence);
        display.setCursor(0, 16);
        display.print("Erros: ");
        display.print(error_count);
        display.display();
        
        delay(2000);  // Pequeno atraso para visualizar a mensagem

        // Envia NACK para solicitar o reenvio do pacote
        sendACK(false);
      }

      // Verifica se todos os pacotes foram recebidos
      if (packet_index >= total_packets) {
        display.clearDisplay();
        display.setCursor(0, 0);
        display.print("Recebido com sucesso!");
        display.display();

        Serial.println("Todos os pacotes recebidos.");
        while (1);  // Pausa após receber todos os pacotes
      }

      start_time = millis();  // Reinicia o timer após receber um pacote
    }
  }

  // Se o tempo de espera ultrapassou, reinicie o receiver
  Serial.println("Tempo limite sem pacotes, reiniciando a recepção.");
}

*/

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    uint8_t packet[packet_size + 3];  // Ajuste de acordo com seu pacote
    LoRa.readBytes(packet, packetSize);
    
    Serial.print("Pacote recebido. Tamanho: ");
    Serial.println(packetSize);

    // Calcule e verifique o checksum
    uint16_t receivedChecksum = packet[packetSize - 1];
    uint16_t calculatedChecksum = calculateChecksum(packet, packetSize - 1);
    
    Serial.print("Checksum recebido: ");
    Serial.println(receivedChecksum, HEX);
    Serial.print("Checksum calculado: ");
    Serial.println(calculatedChecksum, HEX);

    if (receivedChecksum == calculatedChecksum) {
      // ACK para o sender
      LoRa.beginPacket();
      LoRa.write(1);  // Envia ACK (1)
      LoRa.endPacket();
      Serial.println("Pacote válido. ACK enviado.");
      // Atualiza o display com a contagem de pacotes recebidos
      packet_index++;
      display.clearDisplay();
      display.setCursor(0, 0);
      display.print("Pacotes recebidos: ");
      display.print(packet_index);
      display.print("/");
      display.print(total_packets);
      display.setCursor(0, 16);
      display.print(error_count);
      display.display();
    } else {
      Serial.println("Checksum inválido! Pacote descartado.");
      // Opcionalmente, enviar NACK (0)
      error_count++;
    }
  }
}

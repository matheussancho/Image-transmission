/*

///////////////////
// SENDER
/////////////////

// Bibliotecas para LoRa e OLED
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "lena_rgb.h"  // Imagem Lenna em bytes (convertido via Python)

// Definir pinos usados pelo módulo LoRa
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26

// Frequência LoRa
#define BAND 915E6

// Definir pinos do OLED
#define OLED_SDA 4
#define OLED_SCL 15
#define OLED_RST 16
#define SCREEN_WIDTH 128   // Largura do display OLED, em pixels
#define SCREEN_HEIGHT 64  // Altura do display OLED, em pixels

// Variáveis globais
// Tamanho do pacote e número total de pacotes
const int packet_size = 240;
const int total_packets = sizeof(lena_rgb) / packet_size + (sizeof(lena_rgb) % packet_size != 0);
uint16_t packet_index = 0;  // Agora usa uint16_t para suportar mais pacotes
uint16_t last_packet_sent = 0;  // Armazena o último pacote enviado

// Inicializar display OLED
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

// Função para calcular checksum (XOR)
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
  delay(2000);

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
  delay(2000);
}

void loop() {
  if (packet_index < total_packets) {
    int bytes_to_send = (packet_index == total_packets - 1) ? (sizeof(lena_rgb) % packet_size) : packet_size;

    // Prepara o pacote
    uint8_t packet[packet_size + 3];  // Com espaço para o checksum e sequência
    packet[0] = (packet_index >> 8) & 0xFF;  // Parte alta da sequência
    packet[1] = packet_index & 0xFF;         // Parte baixa da sequência
    memcpy(packet + 2, lena_rgb + (packet_index * packet_size), bytes_to_send);
    packet[packet_size + 2] = calculateChecksum(packet + 2, bytes_to_send);  // Checksum

    // Envia o pacote
    LoRa.beginPacket();
    LoRa.write(packet, bytes_to_send + 3);  // Envia o pacote
    LoRa.endPacket();

    // Exibe no display qual pacote foi enviado
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Pacote ");
    display.print(packet_index);
    display.print("/");
    display.print(total_packets);
    display.display();

    last_packet_sent = packet_index;  // Armazena o índice do último pacote enviado
    packet_index++;  // Avança para o próximo pacote

    delay(200);  // Pequeno delay para evitar congestionamento
  } else {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Todos pacotes enviados");
    display.display();
    
    while (1);  // Pausar após enviar todos os pacotes
  }
}

*/
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "lena_rgb.h"    // Imagem Lenna em bytes

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

// Definições da transmissão
const int packet_size = 200;
const int total_packets = sizeof(lena_rgb) / packet_size + (sizeof(lena_rgb) % packet_size != 0);
uint16_t packet_index = 0;  // Índice do pacote atual

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

// Função para calcular o checksum
uint16_t calculateChecksum(const uint8_t *data, size_t len) {
  uint16_t checksum = 0;
  for (size_t i = 0; i < len; i++) {
    checksum ^= data[i];  // Operação XOR para checksum
  }
  return checksum;
}

// Função para esperar por ACK ou NACK
bool waitForAck(int timeout) {
  unsigned long start = millis();
  while (millis() - start < timeout) {
    int packetSize = LoRa.parsePacket();
    if (packetSize > 0) {
      int ack = LoRa.read();  // Ler ACK (1) ou NACK (0)
      return ack == 1;
    }
  }
  return false;
}

void setup() {
  Serial.begin(115200);

  // Inicializar OLED
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);

  Wire.begin(OLED_SDA, OLED_SCL);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C, false, false);
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Inicializando LoRa...");
  display.display();
  delay(10000);

  // Inicializar LoRa
  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DIO0);

  if (!LoRa.begin(BAND)) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Falha no LoRa");
    display.display();
    delay(10000);
    while (1);
  }

  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("LoRa Inicializado");
  display.display();

}


/*
void loop() {
  if (packet_index < total_packets) {
    int bytes_to_send = (packet_index == total_packets - 1) ? (sizeof(lena_rgb) % packet_size) : packet_size;

    uint8_t packet[packet_size + 3];  // Pacote: índice (2 bytes) + dados + checksum
    packet[0] = packet_index & 0xFF;  // Parte baixa do índice
    packet[1] = (packet_index >> 8) & 0xFF;  // Parte alta do índice
    memcpy(packet + 2, lena_rgb + (packet_index * packet_size), bytes_to_send);
    packet[bytes_to_send + 2] = calculateChecksum(packet, bytes_to_send + 2);  // Checksum inclui índice e dados

    Serial.print("Enviando pacote: ");
    Serial.println(packet_index);
    Serial.print("Checksum enviado: ");
    Serial.println(packet[bytes_to_send + 2], HEX);

    bool ack_received = false;
    int resend_attempts = 0;  // Contador de tentativas de reenvio
    const int max_resends = 5;  // Número máximo de reenvios permitidos
    unsigned long start_time = millis();  // Marca o tempo de início

    while (!ack_received && resend_attempts < max_resends) {
      LoRa.beginPacket();
      LoRa.write(packet, bytes_to_send + 3);  // Enviar pacote completo
      LoRa.endPacket();
      
      // Atualiza o display com o status de envio
      display.clearDisplay();
      display.setCursor(0, 0);
      display.print("Pacote ");
      display.print(packet_index + 1);
      display.print("/");
      display.print(total_packets);
      display.display();

      ack_received = waitForAck(1000);  // Espera por ACK ou timeout de 1 segundo
      if (!ack_received) {
        resend_attempts++;
        Serial.println("ACK não recebido, tentando reenviar...");

        display.clearDisplay();
        display.setCursor(0, 0);
        display.print("Reenviando pacote ");
        display.print(packet_index + 1);
        display.setCursor(10, 20);
        display.print("Tentativa: ");
        display.print(resend_attempts);
        display.display();

        delay(1000);  // Espera antes de tentar reenviar
      }

      // Timeout geral de 5 segundos para reenvio de pacotes
      if (millis() - start_time > 5000) {
        Serial.println("Timeout: passando para o próximo pacote.");
        break;  // Sai do loop de reenvio
      }
    }

    // Se não recebeu ACK após várias tentativas, avança para o próximo pacote
    if (!ack_received) {
      Serial.println("Falha ao enviar o pacote após várias tentativas.");
    }
    
    packet_index++;  // Avança para o próximo pacote
    delay(1000);  // Pequeno atraso para evitar congestionamento
  } else {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Todos pacotes enviados");
    display.display();
    while (1);
  }
}
*/

void loop() {
  if (packet_index < total_packets) {
    int bytes_to_send = (packet_index == total_packets - 1) ? (sizeof(lena_rgb) % packet_size) : packet_size;

    uint8_t packet[packet_size + 3];  // Pacote: índice (2 bytes) + dados + checksum
    packet[0] = packet_index & 0xFF;  // Parte baixa do índice
    packet[1] = (packet_index >> 8) & 0xFF;  // Parte alta do índice
    memcpy(packet + 2, lena_rgb + (packet_index * packet_size), bytes_to_send);
    packet[bytes_to_send + 2] = calculateChecksum(packet, bytes_to_send + 2);  // Checksum inclui índice e dados

    Serial.print("Enviando pacote: ");
    Serial.print(packet_index);
    Serial.print(", Checksum enviado: ");
    Serial.println(packet[bytes_to_send + 2], HEX);

    bool ack_received = false;
    int resend_attempts = 0;  // Contador de tentativas de reenvio
    const int max_resends = 5;  // Número máximo de reenvios permitidos
    unsigned long start_time = millis();  // Marca o tempo de início

    // Atualiza o display antes de começar a enviar
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Enviando pacote: ");
    display.print(packet_index);
    display.print("/");
    display.print(total_packets);
    display.setCursor(0, 10);
    display.display();

    while (!ack_received && resend_attempts < max_resends) {
      LoRa.beginPacket();
      LoRa.write(packet, bytes_to_send + 3);  // Enviar pacote completo
      LoRa.endPacket();
      
      Serial.println("Pacote enviado. Aguardando ACK...");

      ack_received = waitForAck(1000);  // Espera por ACK ou timeout de 1 segundo
      if (!ack_received) {
        resend_attempts++;
        Serial.print("ACK não recebido, tentando reenviar... (Tentativa ");
        Serial.print(resend_attempts);
        Serial.print(" de ");
        Serial.print(max_resends);
        Serial.println(")");

        // Atualiza o display com o número de tentativas
        display.setCursor(0, 16);
        display.print("Tentativas: ");
        display.print(resend_attempts);
        display.display();
        delay(1000); // Espera antes de tentar reenviar
      }
      
      // Se o tempo de espera ultrapassou 5 segundos, avance para o próximo pacote
      if (millis() - start_time > 5000) {
        Serial.println("Timeout: passando para o próximo pacote.");
        delay(1000);
        break;  // Sai do loop de reenvio
      }
    }

    if (!ack_received) {
      Serial.println("Falha ao enviar o pacote após várias tentativas.");
      packet_index++;  // Avança para o próximo pacote, mesmo se o atual falhou
    } else {
      packet_index++;
      delay(1000);  // Pequeno atraso para evitar congestionamento
    }
  } else {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Todos pacotes enviados");
    display.display();
    Serial.println("Todos os pacotes foram enviados com sucesso.");
    while (1);
  }
}


///////////////////
//////SENDER//////
/////////////////
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "lena_bytes.h"    // Imagem Lenna em bytes
#include <WiFi.h>       // webserver
#include <WebServer.h> // webserver
#include <esp_system.h>  // Para obter informações do sistema (CPU, memória)
#include <esp_spi_flash.h> // Para obter informações da memória flash (memória de programa que é usada para armazenar o código e dados permanentes)

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
const int total_packets = sizeof(lena_bytes) / packet_size + (sizeof(lena_bytes) % packet_size != 0);
uint16_t packet_index = 0;  // Índice do pacote atual

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

/////////////////// WEBSERVER ////////////////////
const char* ssid = "APTO_2";  // Insira seu SSID
const char* password = "ozzyechanel";  // Insira sua senha WiFi

IPAddress ip(192, 168, 1, 220);  // IP estático escolhido
IPAddress gw(192, 168, 1, 1);    // Gateway (endereço do roteador)
IPAddress subnet(255, 255, 255, 0);  // Máscara de sub-rede

WebServer server(80);  // HTTP Server

unsigned long startTime;
unsigned long totalBytesSent = 0;
unsigned int totalPacketsLost = 0;
unsigned int totalPacketsSent = 0;
unsigned long lastPacketTime = 0;

//////////////////////////////////////////////

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

/////////////////// WEBSERVER - PROMETHEUS //////////////////// 

// Função para obter o uso da memória Flash //////////////////
String getFlashMetrics() {
  String flash_metrics = "";

  // Tamanho total do chip de flash (em bytes)
  size_t totalFlash = ESP.getFlashChipSize();
  
  // Espaço livre disponível para o código/programa (em bytes)
  size_t freeSketchSpace = ESP.getFreeSketchSpace();

  // Total de espaço SPIFFS usado
  size_t usedFlash = totalFlash - freeSketchSpace;

  flash_metrics += "# HELP esp32_flash_usage Flash memory usage\n";
  flash_metrics += "# TYPE esp32_flash_usage gauge\n";
  flash_metrics += "esp32_flash_usage " + String(usedFlash) + "\n";

  flash_metrics += "# HELP esp32_flash_total Total flash memory\n";
  flash_metrics += "# TYPE esp32_flash_total gauge\n";
  flash_metrics += "esp32_flash_total " + String(totalFlash) + "\n";

  flash_metrics += "# HELP esp32_flash_free Free flash memory\n";
  flash_metrics += "# TYPE esp32_flash_free gauge\n";
  flash_metrics += "esp32_flash_free " + String(freeSketchSpace) + "\n";

  return flash_metrics;
}

///// fim da memória flash infos


// Função para calcular a utilização da memória heap RAM
unsigned long getFreeHeap() {
  return esp_get_free_heap_size();
}

// Função para retornar a frequência da CPU
//unsigned long getCpuFrequency() {
//  return (unsigned long)ESP.getCpuFreqMHz();
//}

// Função para calcular a latência
unsigned long calculateLatency() {
  return millis() - lastPacketTime;
}

// Função para calcular a taxa de perda de pacotes
float calculatePacketLossRate() {
  return totalPacketsSent == 0 ? 0 : (float)totalPacketsLost / totalPacketsSent;
}

// Função para calcular a velocidade de transmissão
float calculateTransmissionSpeed() {
  unsigned long elapsedTime = (millis() - startTime) / 1000;
  return elapsedTime > 0 ? totalBytesSent / elapsedTime : 0;
}

// Função para retornar as métricas em formato Prometheus
String getMetrics() {
  String p = "";
  p += "# HELP esp32_transmission_latency Transmission latency\n";
  p += "# TYPE esp32_transmission_latency gauge\n";
  p += "esp32_transmission_latency " + String(calculateLatency()) + "\n";

  p += "# HELP esp32_packet_loss_rate Packet loss rate\n";
  p += "# TYPE esp32_packet_loss_rate gauge\n";
  p += "esp32_packet_loss_rate " + String(calculatePacketLossRate()) + "\n";

  p += "# HELP esp32_transmission_speed Transmission speed\n";
  p += "# TYPE esp32_transmission_speed gauge\n";
  p += "esp32_transmission_speed " + String(calculateTransmissionSpeed()) + "\n";

  p += "# HELP esp32_free_heap Free heap memory\n";  // Memória livre
  p += "# TYPE esp32_free_heap gauge\n";
  p += "esp32_free_heap " + String(getFreeHeap()) + "\n";

  // Adicione as métricas de Flash
  p += getFlashMetrics();

//  p += "# HELP esp32_cpu_frequency CPU frequency in MHz\n";  // Frequência da CPU
//  p += "# TYPE esp32_cpu_frequency gauge\n";
//  p += "esp32_cpu_frequency " + String(getCpuFrequency()) + "\n";

  return p;
}
// Inicializa o WebServer
void startWebServer() {
  server.on("/", []() {
    server.send(200, "text/plain", "ESP32 Metrics Server");
  });

  server.on("/metrics", []() {
    server.send(200, "text/plain", getMetrics());
  });

  server.begin();
  Serial.println("HTTP server started");
}

////////////////////////////////////////////////

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

 ////////// Inicializar WiFi e servidor HTTP //////
  WiFi.config(ip, gw, subnet);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi conectado!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  startWebServer();
  startTime = millis();

//////////////////////////////////////////////////

}


void loop() {
if (packet_index < total_packets) {
    int bytes_to_send = (packet_index == total_packets - 1) ? (sizeof(lena_bytes) % packet_size) : packet_size;

    uint8_t packet[packet_size + 3];  // Pacote: índice (2 bytes) + dados + checksum
    packet[0] = packet_index & 0xFF;  // Parte baixa do índice
    packet[1] = (packet_index >> 8) & 0xFF;  // Parte alta do índice
    memcpy(packet + 2, lena_bytes + (packet_index * packet_size), bytes_to_send);
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
      totalBytesSent += bytes_to_send;  // Incrementa o total de bytes enviados
      totalPacketsSent++;  // Incrementa o total de pacotes enviados
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

  server.handleClient();  // Lidar com as requisições HTTP
}

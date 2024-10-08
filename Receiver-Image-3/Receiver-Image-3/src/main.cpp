/* TESTANDO COM O WEBSERVER TAMBÉM

#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <WebServer.h>
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

// WiFi Configurações
const char* ssid = "SEU_SSID";  // Insira seu SSID
const char* password = "SUA_SENHA";  // Insira sua senha WiFi
IPAddress ip(192, 168, 0, 171);  // IP estático
IPAddress gw(192, 168, 0, 1);    // Gateway
IPAddress subnet(255, 255, 255, 0);  // Subnet
WebServer server(80);  // HTTP Server

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

// Função para retornar as métricas em formato Prometheus
String getMetrics() {
  String p = "";
  p += "# HELP esp32_packets_received Pacotes recebidos\n";
  p += "# TYPE esp32_packets_received counter\n";
  p += "esp32_packets_received " + String(packet_index) + "\n";

  p += "# HELP esp32_packets_errors Pacotes com erro\n";
  p += "# TYPE esp32_packets_errors counter\n";
  p += "esp32_packets_errors " + String(error_count) + "\n";

  return p;
}

// Inicializa o WebServer
void startWebServer() {
  server.on("/", []() {
    server.send(200, "text/plain", "ESP32 Receiver Metrics Server");
  });

  server.on("/metrics", []() {
    server.send(200, "text/plain", getMetrics());
  });

  server.begin();
  Serial.println("HTTP server started");
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

  // Inicializar WiFi e servidor HTTP
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
}

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
      sendACK(true);
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
      display.print("Erros: ");
      display.print(error_count);
      display.display();
    } else {
      Serial.println("Checksum inválido! Pacote descartado.");
      // Envia NACK para o sender
      sendACK(false);
      error_count++;
    }
  }

  server.handleClient();  // Lidar com as requisições HTTP
}
*/


// ----------------------------------------------------------------------------------------

///////////////////
//    RECEIVER   //
///////////////////
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "lena_bytes.h"  // Imagem Lenna em bytes

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
const int total_packets = sizeof(lena_bytes) / packet_size + (sizeof(lena_bytes) % packet_size != 0);
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

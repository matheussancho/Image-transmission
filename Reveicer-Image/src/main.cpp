///////////////////
//////RECEIVER////
/////////////////

//Libraries for LoRa
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//define the pins used by the LoRa transceiver module
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26

//433E6 for Asia
//866E6 for Europe
//915E6 for North America
#define BAND 915E6

#define OLED_SDA 4
#define OLED_SCL 15
#define OLED_RST 16
#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

const int packet_size = 240;
const int total_packets = 640;  // in fact is 68 packets... Adjust according the image size
uint8_t packet_index = 0;
bool all_packets_received = true;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

uint8_t calculateChecksum(const uint8_t *data, size_t len) {
  uint8_t checksum = 0;
  for (size_t i = 0; i < len; i++) {
    checksum ^= data[i];
  }
  return checksum;
}

void setup() {
  //initialize Serial Monitor
  Serial.begin(115200);

  //reset OLED display via software
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);

  //initialize OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C, false, false); // Address 0x3C for 128x32
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Inicializando LoRa...");
  display.display();

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
  
  if (packetSize == (packet_size + 2)) {  // Verifica o tamanho correto do pacote recebido
    uint8_t packet[packet_size + 2];
    LoRa.readBytes(packet, packetSize);

    // Integrity
    uint8_t sequence = packet[0];
    uint8_t checksum_received = packet[packet_size + 1];
    uint8_t checksum_calculated = calculateChecksum(packet + 1, packet_size);

    if (checksum_received == checksum_calculated && sequence == packet_index) {
      Serial.print("Pacote ");
      Serial.print(sequence);
      Serial.println(" recebido corretamente.");

      packet_index++;

      // Atualiza o display com o progresso da recepção
      display.clearDisplay();
      display.setCursor(0, 0);
      display.print("Pacote ");
      display.print(sequence);
      display.print(" recebido.");
      display.display();
    } else {
      Serial.println("Erro: Checksum incorreto ou sequência incorreta.");
      all_packets_received = false;
      
      display.clearDisplay();
      display.setCursor(0, 0);
      display.print("Erro no pacote ");
      display.print(sequence);
      display.display();
    }

    if (packet_index >= total_packets) {
      display.clearDisplay();
      display.setCursor(0, 0);
      if (all_packets_received) {
        display.print("Recebido com sucesso!");
      } else {
        display.print("Erros detectados!");
      }
      display.display();
      
      Serial.println("Todos os pacotes recebidos.");
      while (1);  // Pare após o término da recepção
    }
  }
}

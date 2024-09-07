///////////////////
//////SENDER//////
/////////////////

//Libraries for LoRa
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "lena_bytes.h"  // Lenna image in bytes (python code convert)

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

//OLED pins
#define OLED_SDA 4
#define OLED_SCL 15
#define OLED_RST 16
#define SCREEN_WIDTH 128   // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

const int packet_size = 240;
const int total_packets = sizeof(lena_bytes) / packet_size + (sizeof(lena_bytes) % packet_size != 0);
int packet_index = 0;

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
    int bytes_to_send = (packet_index == total_packets - 1) ? (sizeof(lena_bytes) % packet_size) : packet_size;
    
    // Integrity
    uint8_t packet[packet_size + 2];
    packet[0] = packet_index; 
    memcpy(packet + 1, lena_bytes + (packet_index * packet_size), bytes_to_send);
    packet[bytes_to_send + 1] = calculateChecksum(packet + 1, bytes_to_send);

    LoRa.beginPacket();
    LoRa.write(packet, bytes_to_send + 2);
    LoRa.endPacket();

    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Pacote ");
    display.print(packet_index);
    display.print(" enviado.");
    display.display();

    packet_index++;
    delay(100);  // delay of sender package
  } else {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Todos pacotes enviados");
    display.display();
    while (1);
  }
}

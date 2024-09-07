// Libraries for LoRa
#include <SPI.h>
#include <LoRa.h>

// Libraries for OLED Display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Define the pins used by the LoRa transceiver module
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

// OLED pins
#define OLED_SDA 4
#define OLED_SCL 15
#define OLED_RST 16
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

void setup() {
  Serial.begin(115200);

  // Initialize OLED
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);

  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C, false, false)) {
    Serial.println(F("OLED allocation failed"));
    while (1);
  }

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Inicializando LoRa...");
  display.display();

  // Initialize LoRa
  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DIO0);

  if (!LoRa.begin(BAND)) {
    Serial.println("Falha ao inicializar LoRa!");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Falha no LoRa");
    display.display();
    while (1);
  }

  Serial.println("LoRa inicializado com sucesso.");
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("LoRa inicializado");
  display.display();
  delay(2000);
}

void loop() {
  // Envia uma string simples
  String message = "Teste de comunicação LoRa!";
  
  LoRa.beginPacket();
  LoRa.print(message);
  LoRa.endPacket();

  Serial.print("Enviando mensagem: ");
  Serial.println(message);

  // Exibe no OLED
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Mensagem enviada:");
  display.setCursor(0, 10);
  display.print(message);
  display.display();

  delay(1000); // Espera um segundo antes de enviar novamente
}

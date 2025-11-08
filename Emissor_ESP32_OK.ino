#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>  // Para esp_wifi_set_channel

#include "Wire.h"
#include "I2Cdev.h"
#include "MPU6050.h"

MPU6050 mpu;
int16_t ax, ay, az;
int16_t gx, gy, gz;

// MAC do receptor
// uint8_t broadcastAddress[] = {0xD0, 0xEF, 0x76, 0x15, 0x17, 0x00};
// uint8_t broadcastAddress[] = {0xD0, 0xEF, 0x76, 0x15, 0x69, 0x60};
// uint8_t broadcastAddress[] = {0xD0, 0xEF, 0x76, 0x15, 0x38, 0x00};
uint8_t broadcastAddress[] = {0xD4, 0x8A, 0xFC, 0x9E, 0x53, 0x18};
// uint8_t broadcastAddress[] = {0xD0, 0xEF, 0x76, 0x15, 0x30, 0x58};
// uint8_t broadcastAddress[] = {0xD0, 0xEF, 0x76, 0x15, 0x95, 0x28};

// Estrutura enviada (mantenho igual ao seu código)
typedef struct struct_data {
  String state;
  byte velocidade;
  byte servo;
} struct_data;

struct_data data;

// Botão
#define BUTTONB 14

// === Callback de envio (assinatura nova no core 3.x) ===
void onDataSent(const wifi_tx_info_t *info, esp_now_send_status_t status) {
  Serial.print("Status dos dados enviados: ");
  if (status == ESP_NOW_SEND_SUCCESS) {
    Serial.println("Envio ok!");
  } else {
    Serial.println("Falha no envio!");
  }
}

void setup() {
  delay(5000);
  Serial.begin(115200);

  Wire.begin();
  mpu.initialize();

  pinMode(BUTTONB, INPUT_PULLUP);
  delay(250);

  // Wi-Fi station e canal
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  esp_wifi_set_channel(6, WIFI_SECOND_CHAN_NONE);
  delay(3000);

  // ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Erro ao inicializar o ESP-NOW.");
    return;
  } else {
    Serial.println("Controle conectado com sucesso!");
  }
  delay(3000);

  // Registra callback (assinatura nova)
  esp_now_register_send_cb(onDataSent);

  // Configura peer
  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 6;
  peerInfo.encrypt = false;
  peerInfo.ifidx = WIFI_IF_STA;  // importante no core novo

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Falha ao adicionar o peer");
    return;
  }
}

void loop() {
  delay(50);

  // Botão
  if (digitalRead(BUTTONB) == LOW) {
    data.servo = 1;
  } else {
    data.servo = 0;
  }

  // Leitura do MPU
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  byte x = map(ax, -17000, 17000, 0, 100);
  Serial.print("Angle X: ");
  Serial.println(x);

  byte y = map(ay, -17000, 17000, 100, 200);
  Serial.print("Angle Y: ");
  Serial.println(y);

  Serial.println("");

  // Direção
  const byte ZM = 20;

  if (x < 50 - ZM && y < 150 - ZM) {
    data.state = "q";
  } else if (x < 50 - ZM && y > 150 - ZM && y < 150 + ZM) {
    data.state = "w";
  } else if (x < 50 - ZM && y > 150 + ZM) {
    data.state = "e";
  } else if (y < 150 - ZM && x > 50 - ZM && x < 50 + ZM) {
    data.state = "a";
  } else if (x > 50 - ZM && x < 50 + ZM && y > 150 - ZM && y < 150 + ZM) {
    data.state = "s";
  } else if (y > 150 + ZM && x > 50 - ZM && x < 50 + ZM) {
    data.state = "d";
  } else if (x > 50 + ZM && y < 150 - ZM) {
    data.state = "z";
  } else if (x > 50 + ZM && y > 150 - ZM && y < 150 + ZM) {
    data.state = "x";
  } else if (x > 50 + ZM && y > 150 + ZM) {
    data.state = "c";
  }

  // Velocidade
  byte v = 0;
  if (data.state == "q" || data.state == "e" || data.state == "z" || data.state == "c") {
    v = (abs(x - 50) + abs(x - 150)) / 2; // 0..100%
    v = map(v, 0, 50, 0, 255);
  } else if (data.state == "w" || data.state == "x") {
    v = abs(x - 50); // 0..100%
    v = map(v, 0, 50, 0, 255);
  } else if (data.state == "a" || data.state == "d") {
    v = abs(y - 150); // 0..100%
    v = map(v, 0, 50, 0, 255);
  }
  data.velocidade = v;

  // Debug
  Serial.print("Saida: ");
  Serial.print(data.state);
  Serial.print(" -> ");
  Serial.println(v);

  // Envia
  esp_now_send(broadcastAddress, (uint8_t*)&data, sizeof(data));
}

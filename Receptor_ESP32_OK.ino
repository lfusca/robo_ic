
// Biblioteca para usar a fita de LED
#include <Adafruit_NeoPixel.h>

#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>  // Adicione esta linha para usar esp_wifi_set_channel

#ifdef _AVR_
#include <avr/power.h>
#endif

typedef struct struct_data {
  String state;
  byte velocidade;
  byte garra;
} struct_data;

struct_data data;

// Set your new MAC Address (opcional, o ESP32 já define seu MAC address automaticamente)
// uint8_t newMACAddress[] = {0xBC, 0xDD, 0xC2, 0x16, 0xC9, 0x52};

// Callback executada quando a mensagem for recebida
void onDataRecv(const esp_now_recv_info_t *recvInfo, const uint8_t *incomingData, int len) {
  memcpy(&data, incomingData, sizeof(data));
  const uint8_t* mac = recvInfo->src_addr; // Caso ainda queira acessar o MAC
}

// Define os pinos de utilização do Driver L298N.
int IN1 = 18;
int IN2 = 19;
int IN3 = 26;
int IN4 = 25;

int LED = 15;
#define NUMPIXELS 18  // Quantidade de luzes da fita

Adafruit_NeoPixel pixels(NUMPIXELS, LED, NEO_GRB + NEO_KHZ800);
uint32_t colorValue;


void setup() {
  Serial.begin(115200);
  delay(5000);
  Serial.println("ROBO INICIADO");

  pixels.begin();             // Inicializar NeoPixel
  pixels.setBrightness(255);  // Configurar o brilho dos LEDs (0 a 255)

  // Inicializa os pinos de controle do L298N
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  delay(250);

  // Define a cor dos LEDs para magenta (R:255, G:0, B:130)
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(255, 0, 130));
  }
  pixels.show();

  // Coloca o dispositivo no modo Wi-Fi Station
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();                 // Desconectar de qualquer rede existente
  esp_wifi_set_channel(6, WIFI_SECOND_CHAN_NONE); // Define o canal explicitamente

  // Se você quiser alterar o MAC address, utilize as funções específicas do ESP32, mas não é necessário.
  Serial.print("[ESP32] MAC Address: ");
  Serial.println(WiFi.macAddress());

  // Inicializa o ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Erro ao inicializar o ESP-NOW.");
    return;
  } else {
    Serial.println("Tudo pronto para começar!");
  }

  esp_now_register_recv_cb(onDataRecv);
}

void loop() {

  Serial.print("Status: ");
  Serial.print(data.state);
  Serial.print(" - Velocidade: ");
  Serial.print(data.velocidade);
  Serial.print(" - Garra: ");
  Serial.println(data.garra);

  if (data.state == "w") {
    moveForward();
  } else if (data.state == "x") {
    moveBackward();
  } else if (data.state == "a") {
    moveLeft();
  } else if (data.state == "d") {
    moveRight();
  } else if (data.state == "s") {
    stop();
  }
  // else if (data.state == "q") {
  //   moveForwardLeft();
  // }
  // else if (data.state == "e") {
  //   moveForwardRight();
  // }
  // else if (data.state == "z") {
  //   moveBackwardLeft();
  // }
  // else if (data.state == "c") {
  //   moveBackwardRight();
  // }
}

void moveForward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void moveBackward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void moveLeft() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void moveRight() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void stop() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

void moveForwardLeft() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

void moveForwardRight() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void moveBackwardLeft() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

void moveBackwardRight() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}
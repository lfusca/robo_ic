#include <WiFi.h>
#include <esp_now.h>
#include <Wire.h>
#include <MPU6050_light.h>

// MAC Address do ESP32 Receptor (Robô)
uint8_t broadcastAddress[] = {0xD4, 0x8A, 0xFC, 0x9E, 0x53, 0x18};

// Conexões padrão I2C para ESP32: SDA -> GPIO 21, SCL -> GPIO 22
MPU6050 mpu(Wire);

// Estrutura para envio de dados via ESP-NOW
typedef struct struct_message {
    char tipo[15];      // "INCLINACAO" ou "SOCO"
    char comando[50];   // Direção/Estado: "DIREITA", "FRENTE e ESQUERDA", etc.
} struct_message;

struct_message myData;

// Variável para armazenar o último estado de inclinação enviado
String ultimo_estado_enviado = "Nivelado/Estavel";

// Variáveis de tempo e limites
unsigned long timer_inclinacao = 0;
unsigned long timer_soco_delay = 0;

// --- Parametros de Deteccao ---
const int limite_angulo = 25; // Limite de inclinação em graus (para Roll e Pitch)
const float limite_aceleracao_g = 1.5; // Limite de aceleração em G's (para Soco)
const int delay_soco_ms = 500; // Tempo de espera após um soco para evitar detecção múltipla


// *********************
// FUNÇÃO DE CALLBACK CORRIGIDA
// *********************
void OnDataSent(const wifi_tx_info_t *tx_info, esp_now_send_status_t status) {
  Serial.print("\nStatus do Envio: ");
  if (status == ESP_OK) {
    Serial.println("Sucesso");
  } else {
    Serial.println("Falha no Envio!");
  }
}
// *********************


// Função para enviar o pacote ESP-NOW
void enviarPacote(const char* tipo, const char* comando) {
    strncpy(myData.tipo, tipo, sizeof(myData.tipo));
    strncpy(myData.comando, comando, sizeof(myData.comando));
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

    Serial.print(" >> PACOTE ENVIADO: Tipo: ");
    Serial.print(tipo);
    Serial.print(", Comando: ");
    Serial.println(comando);
}

void setup() {
  Serial.begin(115200);

  // --- CONFIGURAÇÃO MPU6050 ---
  Wire.begin(); 
  byte status = mpu.begin();
  Serial.print(F("Status MPU6050: "));
  Serial.println(status);
  
  while (status != 0) { 
    Serial.println(F("Falha ao encontrar MPU6050!"));
    delay(1000);
  } 
  Serial.println(F("MPU6050 Encontrado. Calibrando... NAO MOVA O SENSOR!"));
  mpu.calcOffsets(true, true); 
  Serial.println("Calibracao concluida.");
  
  // --- CONFIGURAÇÃO ESP-NOW ---
  WiFi.mode(WIFI_STA); // Deve ser configurado como Station
  
  if (esp_now_init() != ESP_OK) {
    Serial.println("Erro ao inicializar ESP-NOW");
    return;
  }
  
  esp_now_register_send_cb(OnDataSent); 
  
  // Adiciona o par (o receptor)
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  
  // *********************
  // CORREÇÃO DO ERRO "Peer interface is invalid"
  // *********************
  peerInfo.ifidx = WIFI_IF_STA; // Especifica a interface STA
  // *********************
  
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Falha ao adicionar peer (receptor)");
    return;
  }
  Serial.println("ESP-NOW inicializado e peer adicionado.");

  timer_inclinacao = millis();
}

void loop() {
  mpu.update(); 
  
  // =================================================================
  // 1. DETECÇÃO E ENVIO DE SOCO (Aceleração)
  // =================================================================
  
  if (millis() < timer_soco_delay) {
    return;
  }
  
  float acX = mpu.getAccX();
  float acY = mpu.getAccY();
  float acZ = mpu.getAccZ();

  float max_ac = 0.0;
  String direcao_soco = "";

  // Lógica de detecção de soco...
  if (abs(acX) > limite_aceleracao_g) {
      max_ac = abs(acX);
      direcao_soco = (acX > 0) ? "DIREITA" : "ESQUERDA";
  }
  if (abs(acY) > limite_aceleracao_g && abs(acY) > max_ac) {
      max_ac = abs(acY);
      direcao_soco = (acY > 0) ? "FRENTE" : "TRAS";
  }
  if (abs(acZ - 1.0) > limite_aceleracao_g && abs(acZ - 1.0) > abs(max_ac - 1.0)) {
      max_ac = abs(acZ); 
      direcao_soco = (acZ > 1.0) ? "PARA CIMA" : "PARA BAIXO";
  }
  
  if (max_ac > 0.0) {
    // * AÇÃO DE ENVIO DO SOCO *
    enviarPacote("SOCO", direcao_soco.c_str());
    
    // Define o timer para ignorar novas detecções de soco por um breve período
    timer_soco_delay = millis() + delay_soco_ms; 
    return; // Não verifica inclinação durante o evento de soco
  }

  // =================================================================
  // 2. DETECÇÃO E ENVIO DE INCLINAÇÃO (Roll e Pitch)
  // =================================================================
  
  if ((millis() - timer_inclinacao) > 100) { 
    timer_inclinacao = millis();

    float angulo_Roll = mpu.getAngleX();
    float angulo_Pitch = mpu.getAngleY();
    
    String orientacao = "Nivelado/Estavel";

    // Lógica de inclinação...
    if (angulo_Roll > limite_angulo) {
      orientacao = "ESQUERDA";
    } else if (angulo_Roll < -limite_angulo) {
      orientacao = "DIREITA";
    } 
    
    if (orientacao == "Nivelado/Estavel") {
      if (angulo_Pitch > limite_angulo) {
        orientacao = "FRENTE";
      } else if (angulo_Pitch < -limite_angulo) {
        orientacao = "TRAS";
      }
    } else {
        // Combina as direções (ex: ESQUERDA e FRENTE)
        if (angulo_Pitch > limite_angulo) {
            orientacao += " e FRENTE";
        } else if (angulo_Pitch < -limite_angulo) {
            orientacao += " e TRAS";
        }
    }
    
    // * AÇÃO DE ENVIO DE INCLINAÇÃO (APENAS SE HOUVER MUDANÇA) *
    if (orientacao != ultimo_estado_enviado) {
        enviarPacote("INCLINACAO", orientacao.c_str());
        ultimo_estado_enviado = orientacao; // Atualiza o último estado enviado
    }
  }
}
#include <WiFi.h>
#include <esp_now.h>
#include <ESP32Servo.h>
// Removemos: #include <esp_wifi.h> // Não é mais necessário

Servo ombroDireito;
Servo antebracoDireito;
Servo cotoveloDireito;

Servo ombroEsquerdo;
Servo antebracoEsquerdo;
Servo cotoveloEsquerdo;

// Estrutura de Dados (DEVE ser idêntica à do Transmissor!)
typedef struct struct_message {
    char tipo[15];      // "INCLINACAO" ou "SOCO"
    char comando[50];   // Direção/Estado: "DIREITA", "FRENTE e ESQUERDA", etc.
} struct_message;

struct_message myData;

// *******************************************************************
// FUNÇÃO DE CALLBACK (RECEBIMENTO)
// *******************************************************************
void OnDataRecv(const esp_now_recv_info *recv_info, const uint8_t *incomingData, int len) {
    const uint8_t * mac_addr = recv_info->src_addr;
    
    memcpy(&myData, incomingData, sizeof(myData));
    
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
             
    Serial.print("---------------------------------------");
    Serial.print("\nPacote Recebido de: ");
    Serial.println(macStr);
    Serial.print("Tamanho do Pacote: ");
    Serial.println(len);

    String direcao = myData.comando;
    Serial.print(">>> COMANDO: ");
    Serial.println(direcao);
    
    String soco = myData.tipo;
    Serial.print(">>> TIPO: ");
    Serial.println(soco);
    
    if(soco == "SOCO"){
      darSocoDuplo();
      iniciar();
    }
    


    Serial.println("---------------------------------------");
}
// *******************************************************************


void darSocoDireito() {
  ombroDireito.write(90);     // 90 centraliza; 180 frente; 0 trás
  antebracoDireito.write(0);  // 0 abaixa; 90 ergue
  cotoveloDireito.write(90);
  delay(500);

  ombroDireito.write(180);
  antebracoDireito.write(90);
  cotoveloDireito.write(0);
  delay(500);

  ombroDireito.write(180);
  antebracoDireito.write(0);
  cotoveloDireito.write(90);
  delay(500);
}

void darSocoEsquerdo() {
  ombroEsquerdo.write(90);     // 90 centraliza; 180 frente; 0 trás
  antebracoEsquerdo.write(180);  // 0 abaixa; 90 ergue
  cotoveloEsquerdo.write(90);
  delay(500);

  ombroEsquerdo.write(0);
  antebracoEsquerdo.write(90);
  cotoveloEsquerdo.write(180);
  delay(500);

  ombroEsquerdo.write(0);
  antebracoEsquerdo.write(180);
  cotoveloEsquerdo.write(90);
  delay(500);
}

void darSocoDuplo(){
  ombroEsquerdo.write(90);     // 90 centraliza; 180 frente; 0 trás
  antebracoEsquerdo.write(180);  // 0 abaixa; 90 ergue
  cotoveloEsquerdo.write(90);
  ombroDireito.write(90);     // 90 centraliza; 180 frente; 0 trás
  antebracoDireito.write(0);  // 0 abaixa; 90 ergue
  cotoveloDireito.write(90);
  delay(500);

  ombroEsquerdo.write(0);
  antebracoEsquerdo.write(90);
  cotoveloEsquerdo.write(180);
  ombroDireito.write(180);
  antebracoDireito.write(90);
  cotoveloDireito.write(0);
  delay(500);

  ombroEsquerdo.write(0);
  antebracoEsquerdo.write(180);
  cotoveloEsquerdo.write(90);
  ombroDireito.write(180);
  antebracoDireito.write(0);
  cotoveloDireito.write(90);
  delay(500);
}

void iniciar() {
  ombroDireito.write(90);
  antebracoDireito.write(0);
  cotoveloDireito.write(90);

  ombroEsquerdo.write(90);
  antebracoEsquerdo.write(180);
  cotoveloEsquerdo.write(90);
}










void setup() {
  Serial.begin(115200);
  delay(1000); 

  ombroDireito.attach(13);
  antebracoDireito.attach(18);
  cotoveloDireito.attach(14);

  ombroEsquerdo.attach(19);
  antebracoEsquerdo.attach(21);
  cotoveloEsquerdo.attach(22);
  iniciar();
  Serial.println("Inicializando ESP32 Receptor...");

  // REMOVIDA: A chamada problemática ao esp_wifi_init()
  
  // 1. Configura o Wi-Fi para o modo Station (necessário para o ESP-NOW)
  // ESTA LINHA JÁ FAZ A INICIALIZAÇÃO DE FATO NA ARDUINO CORE.
  WiFi.mode(WIFI_STA);
  
  // Aumentamos o atraso para dar tempo suficiente ao sistema operacional 
  // para inicializar a interface e ler o MAC.
  delay(200); 
  
  Serial.print("MAC Address (STA) deste ESP32: ");
  Serial.println(WiFi.macAddress());
  
  // 2. Inicializa o ESP-NOW
  if (esp_now_init() != ESP_OK) {
      Serial.println("ERRO: Falha ao inicializar ESP-NOW!");
      return;
  }
  
  Serial.println("ESP-NOW inicializado com sucesso.");

  // 3. Registra a função de callback
  esp_now_register_recv_cb(OnDataRecv); 
}

void loop() {
    // Nada a fazer
}
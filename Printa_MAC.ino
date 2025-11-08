#include <WiFi.h>
#include <esp_wifi.h>

void readMacAddress() {
  uint8_t baseMac[6];
  esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);
  if (ret == ESP_OK) {
    Serial.printf("%02X:%02X:%02X:%02X:%02X:%02X\n",
                  baseMac[0], baseMac[1], baseMac[2],
                  baseMac[3], baseMac[4], baseMac[5]);
  } else {
    Serial.println("Falha ao ler o MAC address");
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);  // Espera inicial para garantir a abertura da porta Serial

  WiFi.mode(WIFI_STA);  // Define o modo Wi-Fi como Station (cliente)

  Serial.print("[DEFAULT] ESP32 MAC Address: ");
  readMacAddress();
}

void loop() {
  // nada aqui
}

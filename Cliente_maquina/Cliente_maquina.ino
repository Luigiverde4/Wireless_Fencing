#include <WiFi.h>

// Datos WiFi
#include <config.h>
const char* IPserver = "192.168.4.1";
int status = WL_IDLE_STATUS;
boolean connected = false;

// Iniciamos el cliente
WiFiClient client;

void setup(){
  Serial.begin(115200);
  Serial.println("CLIENTE");
  connectToWiFi(ssid, password); // Nos conectamos al Wifi
  Serial.printf("Direccion MAC: %s\n", WiFi.macAddress().c_str());
}
void loop() {
}

// Funciones WiFi
void connectToWiFi(const char *ssid, const char *pwd) {
  Serial.println("Connecting to WiFi network: " + String(ssid));
  WiFi.disconnect(true);  // Desconectamos del wifi anterior
  WiFi.onEvent(WiFiEvent);// Detectamos si nos hemos conectado
  WiFi.begin(ssid, pwd);  // Iniciamos WiFi
}

void WiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case 16: // SYSTEM_EVENT_STA_GOT_IP - Nos hemos conectado
      Serial.print("WiFi connected! IP address: ");
      Serial.println(WiFi.localIP());
      connected = true;
      break;
    case 17: // SYSTEM_EVENT_STA_DISCONNECTED - Nos hemos desconectado
      Serial.println("WiFi lost connection");
      connected = false;
      break;
    default: // Otros eventos (handshake etc)
      Serial.println("En el default");
      Serial.println(event);
      break;
  }
}

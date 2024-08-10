#include <WiFi.h>
#include <config.h>

// #define IZQ
#define DER

// Datos conexion
const char* IPserver = "192.168.4.1";
int status = WL_IDLE_STATUS;
unsigned long int id = 1;

// Flags
boolean connected = false;
// UDP
WiFiUDP udp;
int udpPort = 4210;  // int para modificar 

void setup() {
  Serial.begin(115200);
  Serial.println("CLIENTE");

  // Conectar al WiFi
  connectToWiFi(ssid, password); 
  Serial.printf("Direccion MAC: %s\n", WiFi.macAddress().c_str());
  connected = true;

  #if defined(IZQ)
    Serial.println("Tirador: Izquierda");

  #elif defined(DER)
    Serial.println("Tirador: Derecha");
    udpPort = 4211;  // Ahora es válido modificar udpPort

  #else
    Serial.println("TIRADOR NO DEFINIDO, auto IZQ");
    #define IZQ  // Sin punto y coma
  #endif

  // Conectar al servidor UDP
  udp.begin(udpPort);
}

void loop() {
  delay(7); // Con un delay de 10 a 7 habia 0 perdidas. 5 - 20% de perdidas
  // Comprobamos la conexion a WiFi
  if (WiFi.status() != WL_CONNECTED) { // Si el estado NO es conectado
      Serial.println("Estado: NO conectado al WiFi");
      connectToWiFi(ssid, password); // Nos conectamos al wifi
  }
  enviar_datos();
}

// Funciones WiFi
void connectToWiFi(const char *ssid, const char *pwd) {
  /*
    Conectar el dispositivo al WiFi
    ssid: Nombre del wifi
    pwd: Contrasenya del wifi
  */
  Serial.println("Conectando a WiFi : " + String(ssid));
  WiFi.disconnect(true);  // Desconectamos del wifi anterior
  WiFi.onEvent(WiFiEvent); // Detectamos si nos hemos conectado
  WiFi.begin(ssid, pwd);  // Iniciamos WiFi
  delay(1000);            // Delay de 1seg para que le de tiempo
}

void WiFiEvent(WiFiEvent_t event) {
  /*
    Lidiar con los WiFiEvents
    event: Evento WiFI que tratar con
  */
  Serial.printf("En WiFiEvent: %d\n", event);
  switch (event) {
    case 16: // SYSTEM_EVENT_STA_GOT_IP
      Serial.printf("Conectado a WiFi! Direccion IP: %s\n", WiFi.localIP().toString().c_str());
      connected = true;
      break;
    case 17: // SYSTEM_EVENT_STA_DISCONNECTED
      Serial.println("Perdida conexion WiFi");
      connected = false;
      break;
    default: // El default
      id = 1;
      Serial.printf("Otro evento: %d\n", event);
      break;
  }
}

void enviar_datos() {
  // Enviar el tiempo actual al servidor
  udp.beginPacket(IPAddress(192, 168, 4, 1), udpPort);
  udp.printf("ID: %lu\nTICK:%lu\nRSSI: %ld dbm\r", id++,millis(),WiFi.RSSI());
  udp.endPacket();
}
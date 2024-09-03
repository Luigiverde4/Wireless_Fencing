#include <WiFi.h>
#include <config.h>

#define IZQ
// #define DER

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
  //Serial.begin(115200);
  Serial.println("CLIENTE");

  // Conectar al WiFi
  connectToWiFi(ssid, password); 
  Serial.printf("Direccion MAC: %s\n", WiFi.macAddress().c_str());
  connected = true;

  #if defined(IZQ)
    Serial.println("Tirador: Izquierda");

  #elif defined(DER)
    Serial.println("Tirador: Derecha");
    udpPort = 4211;  // Ahora es valido modificar udpPort

  #else
    Serial.println("TIRADOR NO DEFINIDO, auto IZQ");
    #define IZQ  // Sin punto y coma
  #endif

  // Conectar al servidor UDP
  udp.begin(udpPort);
}

void loop() {
  delay(20); // Un paquete recibido cada 0.05 seg como poco
  // Serial.println(traduce_status(WiFi.status()));
  // Comprobamos la conexion a WiFi
  if (WiFi.status() != WL_CONNECTED) { // Si el estado NO es conectado
      //Serial.println("Estado: NO conectado al WiFi");
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
  //Serial.println("Conectando a WiFi : " + String(ssid));
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
  //Serial.printf("En WiFiEvent: %d\n", event);
  switch (event) {
    case 16: // SYSTEM_EVENT_STA_GOT_IP
      //Serial.printf("Conectado a WiFi! Direccion IP: %s\n", WiFi.localIP().toString().c_str());
      connected = true;
      break;
    case 17: // SYSTEM_EVENT_STA_DISCONNECTED
      //Serial.println("Perdida conexion WiFi");
      connected = false;
      break;
    default: // El default
      id = 1;
      //Serial.printf("Otro evento: %s\n", traduce_WiFiEvent(event));
      break;
  }
}

void enviar_datos() {
  // Enviar el tiempo actual al servidor
  udp.beginPacket(IPAddress(192, 168, 4, 1), udpPort);
  udp.printf("ID:%lu\nTICK:%lu\nV0:%d\nV1:%d\r",id++,millis(),0,10); // \nRSSI: %ld dbm,WiFi.RSSI()
  udp.endPacket();
  // Serial.println("Paquete enviado");
}
/*
ID: Nº de pck enviado
Tick - Tiempo en el que se envia
V0: Voltaje de vuelta del florete (Cable C) - Normalmente = X , Tocado = 0
V1: Voltaje de la chaquetilla (Cable A) - Normalmente = 0, Tocado = X
*/

// Traductores para debug 
/*
const char* traduce_status(int status) {
    switch (status) {
        case WL_NO_SHIELD: return "WL_NO_SHIELD";
        case WL_IDLE_STATUS:return "WL_IDLE_STATUS";
        case WL_NO_SSID_AVAIL:return "WL_NO_SSID_AVAIL";
        case WL_SCAN_COMPLETED:return "WL_SCAN_COMPLETED";
        case WL_CONNECTED:return "WL_CONNECTED";
        case WL_CONNECT_FAILED:return "WL_CONNECT_FAILED";
        case WL_CONNECTION_LOST:return "WL_CONNECTION_LOST";
        case WL_DISCONNECTED:return "WL_DISCONNECTED";
        default:return "EN EL DEFAULT";
    }
}

const char* traduce_WiFiEvent(int event) {
    switch (event) {
        case 0: return "SYSTEM_EVENT_WIFI_READY";
        case 1: return "SYSTEM_EVENT_SCAN_DONE";
        case 2: return "SYSTEM_EVENT_STA_START";
        case 3: return "SYSTEM_EVENT_STA_STOP";
        case 4: return "SYSTEM_EVENT_STA_CONNECTED";
        case 5: return "SYSTEM_EVENT_STA_DISCONNECTED";
        case 6: return "SYSTEM_EVENT_STA_AUTHMODE_CHANGE";
        case 7: return "SYSTEM_EVENT_STA_GOT_IP";
        case 8: return "SYSTEM_EVENT_STA_LOST_IP";
        case 9: return "SYSTEM_EVENT_STA_WPS_ER_SUCCESS";
        case 10: return "SYSTEM_EVENT_STA_WPS_ER_FAILED";
        case 11: return "SYSTEM_EVENT_STA_WPS_ER_TIMEOUT";
        case 12: return "SYSTEM_EVENT_STA_WPS_ER_PIN";
        case 13: return "SYSTEM_EVENT_AP_START";
        case 14: return "SYSTEM_EVENT_AP_STOP";
        case 15: return "SYSTEM_EVENT_AP_STACONNECTED";
        case 16: return "SYSTEM_EVENT_AP_STADISCONNECTED";
        case 17: return "SYSTEM_EVENT_AP_PROBEREQRECVED";
        case 18: return "SYSTEM_EVENT_GOT_IP6";
        case 19: return "SYSTEM_EVENT_ETH_START";
        case 20: return "SYSTEM_EVENT_ETH_STOP";
        case 21: return "SYSTEM_EVENT_ETH_CONNECTED";
        case 22: return "SYSTEM_EVENT_ETH_DISCONNECTED";
        case 23: return "SYSTEM_EVENT_ETH_GOT_IP";
        case 24: return "SYSTEM_EVENT_MAX";
        default: return "UNKNOWN_EVENT";
    }
}
*/
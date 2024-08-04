#include <WiFi.h>
#include <config.h>

// Datos Server y conexion
const char* IPserver = "192.168.4.1";
const int port = 80; // Port number for TCP
int status = WL_IDLE_STATUS;
boolean connected = false;

// Cliente
WiFiClient client;
struct {
  String ip;
} cliente_server;
struct {
  int hora;
  String msj;
} mensaje;

void setup() {
  Serial.begin(115200);
  Serial.println("CLIENTE");
  // Conectar  al WiFi
  connectToWiFi(ssid, password); 
  Serial.printf("Direccion MAC: %s\n", WiFi.macAddress().c_str());
  // Conectar al servidor del WiFi
  connectToServer(client);
}

void loop() {
  // Guardar en el struct
  cliente_server.ip = WiFi.localIP().toString();
  // Comprobamos la conexion a WiFi
  if (WiFi.status() != WL_CONNECTED) { // Si el estado NO es conectado
      Serial.println("Estado: NO conectado al WiFi");
      connectToWiFi(ssid, password); // Nos conectamos al wifi
  }

  // Comprobamos la conexion al Server
  if (client.connected()) { // Si estamos conectados al servidor
    client.printf("Han pasado %d\n milisegundos",millis()); // Mandamos datos al server
    delay(3000);
  } else {                  // Estamos conectados al WiFi pero no al servidor
    Serial.println("Cliente NO conectado al server, SI al WiFi");
    client.stop(); // Parar el cliente si nos hemos desconectado (a lo mejor no parar ??)
    connectToServer(client);  
  }
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
}

void WiFiEvent(WiFiEvent_t event) {
  /*
    Lidiar con los WiFiEvents
    event: Evento WiFI que tratar con
  */
  Serial.println("En WiFiEvent");
  switch (event) {
    case 16: // SYSTEM_EVENT_STA_GOT_IP - Conectado al wifi
      Serial.printf("Conectado a WiFi! Direccion IP: %s\n", WiFi.localIP().toString().c_str());
      connected = true;
      break;
    case 17: // SYSTEM_EVENT_STA_DISCONNECTED - Nos hemos desconectado
      Serial.println("Perdida conexion WiFi");
      connected = false;
      break;
    default: // El default
      Serial.printf("Otro evento: %d\n", event);
      break;
  }
}

void connectToServer(WiFiClient &client) {
  /*
    Conectar el dispositivo a un servidor dentro del wfi
    client: Cliente con el que vamos a conectarnos al server 
  */
  if (client.connect(IPserver, port)) { // Nos intentamos conecetar
    Serial.println("Conectado al servidor");
  } else {
    Serial.println("Fallo en la conexion al servidor");
  }
}
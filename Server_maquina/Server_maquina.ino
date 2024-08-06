#include <WiFi.h>

// Datos WiFi
const char* ssid = "ESP32_Server";
const char* password = "12345678";

// Flags
bool primera_vez = true;

// Datos UDP
WiFiUDP udp;
const unsigned int localUDPPort = 4210; // Puerto
char incomingPacket[255]; // Buffer para los paquetes entrantes (ajustar a futuro)
unsigned long int id;

void setup() {
  Serial.begin(115200);
  Serial.println("SERVIDOR UDP");
  // Configuramos el ESP32 como Access Point
  WiFi.softAP(ssid, password);

  // Obtenemos la direccion IP del AP
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("Direccion IP del AP: ");
  Serial.println(myIP);

  // Iniciamos el servidor
  udp.begin(localUDPPort);
}

void loop() {
  // Comprobamos si ha llegado algun paquete UDP
  int packetSize = udp.parsePacket();
  if (packetSize){              // Tamaño paquete != 0 -> Hay paquete
    int len = udp.read(incomingPacket, 255);
    if (len > 0){               // Comprobamos si hay contenido en el paquete
      incomingPacket[len] = 0;  // Ponemos un 0 para poner fin al string
    }
    Serial.printf("Tamaño paquete: %d de %s:%d\n", packetSize, udp.remoteIP().toString().c_str(), udp.remotePort());
    Serial.printf("Contenido: %s\n", incomingPacket);
    //analizar_paquete(incomingPacket):
  }
  delay(2000);
}


void analizar_paquete(const char* incomingPacket) {
  if (primera_vez) {
    sincroReloj();  // Asumiendo que no necesitas pasar la cadena a sincroReloj
  }
  // Codigo de analizar voltaje
}

void sincroReloj() {
  // Sincroniza el reloj con el cliente
  Serial.println("Pendiente por hacer");
}
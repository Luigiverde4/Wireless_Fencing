#include <WiFi.h>

// Datos WiFi
const char* ssid = "ESP32_Server";
const char* password = "12345678";

// Creamos un servidor en el puerto 80
WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  
  // Configuramos el ESP32 como Access Point
  WiFi.softAP(ssid, password);

  // Obtenemos la dirección IP del AP
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("Dirección IP del AP: ");
  Serial.println(myIP);

  // Iniciamos el servidor
  server.begin();
}

void loop() {
  // Comprobamos si se ha conectado algún cliente
  WiFiClient client = server.available();

  if (client) { // Se ha conectado alguien
    Serial.print("IP Cliente conectado: ");
    IPAddress IPCliente = client.remoteIP();
    Serial.println(IPCliente);

    // Procesar la solicitud del cliente aquí
    // Por ejemplo, leer datos del cliente
    while (client.connected()) {
      if (client.available()) {
        String request = client.readStringUntil('\r');
        Serial.print("Request: ");
        Serial.println(request);
        client.flush();
      }
    }

    // Cerrar la conexión
    client.stop();
    Serial.println("Cliente desconectado");
  }
}

#include <WiFi.h>
#include <esp_wifi.h> // Para los dispositivos conectados

// Datos WiFi
const char* ssid = "ESP32_Server";
const char* password = "12345678";

// Creamos un servidor en el puerto 80
WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  Serial.println("SERVIDOR");
  // Configuramos el ESP32 como Access Point
  WiFi.softAP(ssid, password);

  // Obtenemos la direccion IP del AP
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("Direccion IP del AP: ");
  Serial.println(myIP);

  // Iniciamos el servidor
  server.begin();
}

void loop() {
  // Comprobamos si se ha conectado alg�n cliente
  WiFiClient client = server.available();

  if (client) { // Se ha conectado alguien
    Serial.print("IP Cliente conectado: ");
    IPAddress IPCliente = client.remoteIP();
    Serial.println(IPCliente);

    // Procesar la solicitud del cliente aqui
    // Por ejemplo, leer datos del cliente
    while (client.connected()) {
      if (client.available()) {
        String request = client.readStringUntil('\r');
        Serial.print("Request: ");
        Serial.println(request);
        client.flush();
      }
    }

    // Cerrar la conexion
    client.stop();
    Serial.println("Cliente desconectado");
  }
  listConnectedDevices();
  delay(2000);     
}

void listConnectedDevices() {
    wifi_sta_list_t stationList;
    esp_wifi_ap_get_sta_list(&stationList);

    Serial.printf("Numero de dispositivos conectados: %d\n", stationList.num);

    for (int i = 0; i < stationList.num; i++) {
        wifi_sta_info_t station = stationList.sta[i];
        IPAddress ip = WiFi.softAPIP(); // Obtiene la IP del dispositivo conectado

        Serial.printf("Dispositivo %d MAC: %02x:%02x:%02x:%02x:%02x:%02x, IP: %s\n", 
                      i + 1,
                      station.mac[0], station.mac[1], station.mac[2],
                      station.mac[3], station.mac[4], station.mac[5],
                      ip.toString().c_str());
    }
}

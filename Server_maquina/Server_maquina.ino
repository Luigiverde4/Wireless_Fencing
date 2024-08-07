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
unsigned long int perdidos = 0, primer_id = 0,primer_tiempo = 0,tiempo=0; // Perdidos: Nº paq perdidos. . Primer_id = ID del primer paquete que llegue. primer_tiempo = tiempo del primer paquete, tiempo = tiempo en el que recibe el paquete
unsigned long int id = 1,cont = 1;    // ID = id del último paquete Cont = Contador de paquetes recibidos
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
      tiempo = millis();
      cont++;                   // Incrementamos el contador de paquetes recibidos
      incomingPacket[len] = 0;  // Ponemos un 0 para poner fin al string
      analizar_paquete(incomingPacket); 
    }
    /*
      Serial.printf("Tamaño paquete: %d de %s:%d\n", packetSize, udp.remoteIP().toString().c_str(), udp.remotePort());
    */
  }
}


void analizar_paquete(const char* incomingPacket) {
  // Saccar los datos del paquete
  String idStr = obtener_dato(incomingPacket, "ID"); // ID
  id = idStr.toInt();

  if (primera_vez) {
    sincroReloj();
    // Asignamos IDs
    cont = id;
    primer_id = id;

    // Cogemos el tiempo de cuando se recibe el primer paquete
    primer_tiempo = tiempo;
    // Tachamos que sea la primera vez
    primera_vez = false;
  }

  if (idStr.length() > 0) { // Ha encontrado el ID
    if (id > cont) { // Si hemos perdido paquetes
      perdidos += (id - cont); // Calculamos los paquetes perdidos
    } 
    cont = id; // Actualizamos el contador al ID del ultimo paquete recibido
  }

  // Calculamos el porcentaje de paquetes perdidos
  float porcentaje_perdidos = (float)perdidos / (cont - primer_id) * 100.0 ;
  // Calculamos el tiempo transcurrido en segundos
  float tiempo_transcurrido = (tiempo - primer_tiempo) / 1000.0;
  // Calculamos los paquetes por segundo (PPS)
  float pps = (cont - primer_id) / tiempo_transcurrido;

  Serial.printf("Paquetes perdidos: %ld, TOTAL: %ld, PPS: %.2f, Porcentaje perdidos: %.2f%%\n", perdidos, cont, pps, porcentaje_perdidos);

  // Código de analizar voltaje (pendiente de implementar)
}

String obtener_dato(const char* packet, const char* clave) {
  /*
    Saca datos del mensaje enviado por UDP con Strings
    packet: Contenido del paquete como cadena de caracteres
    clave:  Dato que queremos sacar
  */

  String packetStr = String(packet);
  String clave_a_encontrar = String(clave) + ": ";
  
  // Encuentra la posición en la cadena packetStr donde comienza clave_a_encontrar.
  int IndiceInicial = packetStr.indexOf(clave_a_encontrar);
  
  if (IndiceInicial != -1) {// Si clave_a_encontrar es encontrada
    // Ajusta IndiceInicial para que apunte al comienzo del valor asociado a la clave.
    IndiceInicial += clave_a_encontrar.length();
    // Encuentra la posición del siguiente salto de linea
    int IndiceFinal = packetStr.indexOf("\n", IndiceInicial);
    
    // Si no se encuentra el salto de línea, se ajusta IndiceFinal para que sea el final de la cadena.
    if (IndiceFinal == -1) {
      IndiceFinal = packetStr.length();
    }   
    // Devuelve el substring que contiene el valor asociado a la clave.
    return packetStr.substring(IndiceInicial, IndiceFinal);
  }
  // Si la clave no esta en el paquete, return vacio.
  return "";
}

void sincroReloj() {
  // Sincroniza el reloj con el cliente
  Serial.println("Pendiente por hacer");
}
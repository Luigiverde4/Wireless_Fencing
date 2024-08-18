#include <WiFi.h>
#include <maquina.h>

// Datos WiFi
const char* ssid = "ESP32_Server";
const char* password = "12345678";

// Estructura para manejar cada conexion UDP
struct UDPConnection {
  /*
        Paquetes (pck)
  perdidos -> Nº pck perdidos
  primer_id -> ID del 1er pck que llega
  id -> ID del paquete recibido
  cont -> Contador de los pck recibidos
          Tiempo
  primer_tiempo -> Momento en el que llega el 1er pck
  tienpo -> Momento en el que se recibe el pck 
  */
  WiFiUDP udp;
  unsigned int port;
  char incomingPacket[255];
  unsigned long perdidos;
  unsigned long primer_id;
  unsigned long primer_tiempo;
  unsigned long tiempo;
  unsigned long id;
  unsigned long cont;
  bool primera_vez;

  // Constructor
  UDPConnection(unsigned int p) : 
  port(p),
  perdidos(0), 
  primer_id(0), 
  primer_tiempo(0), 
  tiempo(0),
  id(1), 
  cont(1), 
  primera_vez(true) {}
};

// Crear dos instancias de UDPConnection
UDPConnection connection1(4210);
UDPConnection connection2(4211);

void setup() {
  Serial.begin(115200);
  Serial.println("SERVIDOR UDP");

  // Configuramos el ESP32 como Access Point
  WiFi.softAP(ssid, password);

  // Obtenemos la direccion IP del AP
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("Direccion IP del AP: ");
  Serial.println(myIP);

  // Iniciamos los servidores UDP en los puertos correspondientes
  connection1.udp.begin(connection1.port);
  connection2.udp.begin(connection2.port);
}

void loop() {
  recibir_paquete(connection1);
  recibir_paquete(connection2);
}

// Funcion para recibir paquetes de una conexion UDP específica
void recibir_paquete(UDPConnection &conn) {
  // Comprobamos si ha llegado algun paquete UDP
  int packetSize = conn.udp.parsePacket();
  if (packetSize) { // Tamaño paquete != 0 -> Hay paquete
    int len = conn.udp.read(conn.incomingPacket, 255);
    if (len > 0) { // Comprobamos si hay contenido en el paquete
      conn.tiempo = millis();
      conn.cont++; // Incrementamos el contador de paquetes recibidos
      conn.incomingPacket[len] = 0; // Ponemos un 0 para poner fin al string
      analizar_paquete(conn);
    }
  }
}

// Funcion para analizar un paquete específico
void analizar_paquete(UDPConnection &conn) {
  // Extraer los datos del paquete
  String idStr = obtener_dato(conn.incomingPacket, "ID"); // ID
  conn.id = idStr.toInt();

  if (conn.primera_vez) {
    sincroReloj();
    // Asignamos IDs
    conn.cont = conn.id;
    conn.primer_id = conn.id;

    // Tomamos el tiempo de cuando se recibe el primer paquete
    conn.primer_tiempo = conn.tiempo;
    // Marcamos que ya no es la primera vez
    conn.primera_vez = false;
  }

  if (idStr.length() > 0) { // Si ha encontrado el ID
    if (conn.id > conn.cont) { // Si hemos perdido paquetes
      conn.perdidos += (conn.id - conn.cont); // Calculamos los paquetes perdidos
    } 
    conn.cont = conn.id; // Actualizamos el contador al ID del ultimo paquete recibido
  }

  // Calculamos el porcentaje de paquetes perdidos
  float porcentaje_perdidos = (float)conn.perdidos / (conn.cont - conn.primer_id) * 100.0;
  // Calculamos el tiempo transcurrido en segundos
  float tiempo_transcurrido = (conn.tiempo - conn.primer_tiempo) / 1000.0;
  // Calculamos los paquetes por segundo (PPS)
  float pps = (conn.cont - conn.primer_id) / tiempo_transcurrido;

  Serial.printf("Puerto %d - Paquetes perdidos: %ld, TOTAL: %ld, PPS: %.2f, Porcentaje perdidos: %.2f%%\n", 
                conn.port, conn.perdidos, conn.cont, pps, porcentaje_perdidos);

  // Codigo de analizar voltaje (pendiente de implementar)
}

// Funcion para obtener un dato específico del paquete
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
  
    if (IndiceInicial != -1) { // Si clave_a_encontrar es encontrada
        // Ajusta IndiceInicial para que apunte al comienzo del valor asociado a la clave.
        IndiceInicial += clave_a_encontrar.length();
      
        // Encuentra la posición del siguiente salto de línea
        int IndiceFinal = packetStr.indexOf("\n", IndiceInicial);
      
        // Si no se encuentra el salto , pone IndiceFinal para que sea el final de la cadena.
        if (IndiceFinal == -1) {
            IndiceFinal = packetStr.length();
        }   
      
        // Devuelve el substring que contiene el valor asociado a la clave.
        return packetStr.substring(IndiceInicial, IndiceFinal);
    }
  
    // Si la clave no esta en el paquete, return vacio.
    return "";
}

// Reloj
void sincroReloj() {
  // Sincroniza el reloj con el cliente
  Serial.println("Pendiente por hacer");
}

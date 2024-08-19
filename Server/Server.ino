#include <WiFi.h>
#include <maquina.h>

// Maquina
MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);
byte puntaje1 = 0;
byte puntaje2 = 0;
byte minutos = 3;
byte segundos = 0;

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
  unsigned int ultima_medicion;
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
  ultima_medicion(0),
  primera_vez(true) {}
};

// Crear dos instancias de UDPConnection
UDPConnection connection1(4210);
UDPConnection connection2(4211);

void setup() {
  Serial.begin(115200);
  Serial.println("SERVIDOR UDP");

  // Inicializamos la maquina
  mx.begin();
  mx.control(MD_MAX72XX::INTENSITY, MAX_INTENSITY / 2);
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
  mx.clear();  

  // Inicializamos la maquina
  mx.begin();
  mx.control(MD_MAX72XX::INTENSITY, MAX_INTENSITY / 2);
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
  mx.clear();  

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
  // Primero que reciba paquetes
  if (recibir_paquete(connection1) || recibir_paquete(connection2)){
    ponerNumero(1,0);
  }
}

// Funcion para recibir paquetes de una conexion UDP específica
bool recibir_paquete(UDPConnection &conn) {
  // Comprobamos si ha llegado algun paquete UDP
  int packetSize = conn.udp.parsePacket();
  if (packetSize) { // Tamaño paquete != 0 -> Hay paquete
    int len = conn.udp.read(conn.incomingPacket, 255);
    if (len > 0) { // Comprobamos si hay contenido en el paquete
      conn.tiempo = millis();
      conn.cont++; // Incrementamos el contador de paquetes recibidos
      conn.incomingPacket[len] = 0; // Ponemos un 0 para poner fin al string
      return analizar_paquete(conn);
    }
  }
}

// Funcion para analizar un paquete específico
bool analizar_paquete(UDPConnection &conn) {
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

  // Analizar voltaje
  String voltajeStr = obtener_dato(conn.incomingPacket, "V"); // Voltaje
  int voltaje_actual = voltajeStr.toInt();

  // Si la ultima medicion es diferente a la medicion actual, actualizarla
  if (conn.ultima_medicion != voltaje_actual) {
    // Nuevo voltaje
    conn.ultima_medicion = voltaje_actual;
    // CODIGO PARA ANALIZAR SI ES TOCADO VALIDO
    return true;

  } else {
    // El voltaje no ha cambiado
    return false;
  }
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

// FUNCIONES MAQUINA
const byte numeros[10][8] = {
    // 0
    { 
    0b00111100, 
    0b01000100, 
    0b01000100, 
    0b01000100, 
    0b01000100, 
    0b01000100, 
    0b01000100, 
    0b00111100 },

    // 1
    { 
    0b00010000,
    0b00110000,
    0b01010000, 
    0b00010000, 
    0b00010000, 
    0b00010000, 
    0b00010000, 
    0b00111100 },

    // 2
    { 
    0b00111100, 
    0b01000010, 
    0b00000010, 
    0b00011100, 
    0b00100000, 
    0b01000000, 
    0b01000000, 
    0b01111110 },

    // 3

    { 
    0b00111100, 
    0b01000010, 
    0b00000010, 
    0b00111100, 
    0b00000010, 
    0b00000010, 
    0b01000010, 
    0b00111100 },

    { 
    0b00001100, 
    0b00010100, 
    0b00100100, 
    0b01000100, 
    0b01111111, 
    0b00000100, 
    0b00000100, 
    0b00000100 },

    // 5
    { 
    0b01111110, 
    0b01000000, 
    0b01000000, 
    0b01111100, 
    0b00000010, 
    0b00000010, 
    0b01000010, 
    0b00111100 },

    // 6
    { 
    0b00111100, 
    0b01000000, 
    0b01000000, 
    0b01111100, 
    0b01000010, 
    0b01000010, 
    0b00111100, 
    0b00000000 },

    // 7
    { 
    0b01111110, 
    0b00000010, 
    0b00000100, 
    0b00001000, 
    0b00010000, 
    0b00100000, 
    0b01000000, 
    0b01000000 },

    //  8
    { 
    0b00111100, 
    0b01000010,
    0b01000010,
    0b00111100, 
    0b01000010, 
    0b01000010, 
    0b01000010, 
    0b00111100 },

    //  9
    { 
    0b00111100, 
    0b01000010, 
    0b01000010, 
    0b00111110, 
    0b00000010, 
    0b00000010, 
    0b01000010, 
    0b00111100 }
};

void ponerNumero(int numero, int pos_x) {
/*
    int numero: numero a escribir en la matriz 
    int pos_x: posición en donde se escribe el numero

    Escribe el numero en la matriz
*/
    // Comprobamos que el numero está en el rango
    if (numero < 0 || numero > 9) {
        Serial.println("numero fuera de rango");
        return;
    }

    // Dibujamos el numero en la matriz a partir de pos_x
    for (int columna = 0; columna < 8; columna++) { // Recorremos columnas
        for (int fila = 0; fila < 8; fila++) {      // Recorremos las filas de la columna
            // Obtenemos el valor del bit correspondiente de la columna
            bool valor = (numeros[numero][columna] >> fila) & 1;
            // Ponemos el punto en la matriz
            mx.setPoint(pos_x + columna, fila, valor);
        }
    }
}

void ponerPuntos(int puntaje1, int puntaje2) {
    // Copiar los puntos en la matriz
    ponerNumero(puntaje1, 24);    // Puntaje jugador 1,0 ya que es el primer bloque
    ponerNumero(puntaje2, 0);   // Puntaje jugador 2, 24 ya que es el 4to bloque
}
void ponerTiempo(int minutos, int segundos){
    // Mostrar minutos
    ponerNumero(minutos, 15);     // Minutos, comenzando en la posición 8
    
    /*
    // Mitad superior
    int decenas_segundos = segundos / 10; // Decenas de segundos
    for (int fila = 0; fila < 4; fila++) { // Recorremos las 4 filas
        for (int columna = 0; columna < 4; columna++) { // Recorremos las 4 columnas
            // Copiar bits de numeros_chiquitos al arreglo matrix_led para la mitad superior
            [8 + columna][fila] = numeros_chiquitos[decenas_segundos][fila][columna]; // Empezamos desde la columna 18, lo mas arriba posible , fila 0
        }
    }
    
    // Mitad inferior
    int unidades_segundos = segundos % 10; // Segundos
    for (int fila = 0; fila < 4; fila++) { // Recorremos las 4 filas
        for (int columna = 0; columna < 4; columna++) { // Recorremos las 4 columnas
            // Copiar bits de numeros_chiquitos al arreglo matrix_led para la mitad inferior
            [12 + columna][fila + 4] = numeros_chiquitos[unidades_segundos][fila][columna]; // Empezamos desde la columna 20, fila 4
        }
    }
    */
}
void vaciarMatriz() {
/*
    Asigna los valores de las filas de cada columna a 0
*/
    for (int x = 0; x < 32; x++) {
        for (int y = 0; y<8;y++){
            mx.setPoint(y, x, 0); 
        }
    }
}

// Manipular puntos
void manipularPunto(int jugador, int cambio) {
/*
    Incrementa / Decrementa y limita el valor de puntos del tirador
*/
    if (jugador == 1) {
        puntaje1 = constrain(puntaje1 + cambio, 0, 15); // Limitamos el puntaje entre 0 y 15
    } else if (jugador == 2) {
        puntaje2 = constrain(puntaje2 + cambio, 0, 15);
    }
}
void resetPuntos(){
/*
    Resetea los puntos de los dos jugadores a 0
*/
  puntaje1 = 0;
  puntaje2 = 0;
}

// Tiempo
void cuentaAtras(){
/*
    Actualiza el transcurso del tiempo del marcador
*/
    static unsigned long tiempo_previo = 0;
    unsigned long tiempo_pasado = millis();
    const static long intervalo = 1000;

    if (segundos == 0 && minutos != 0){ // No es el ultimo minuto
    segundos = 59;
    minutos--;
    }else if(segundos == 0 && minutos == 0){ // Se ha acabado el tiempo
    segundos = 0;
    minutos = 0;
    tone(7, 1000, 300); // Utilizando el pin 7 para el altavoz
    return;
    }

    if (tiempo_pasado - tiempo_previo >= intervalo) { // Ha pasado 1seg ?
    tiempo_previo = tiempo_pasado;
    segundos--;
    }
}

void resetTiempo(){
/*
    Resetea el tiempo a 3min y 0seg
*/
    minutos = 3;
    segundos = 0;
}

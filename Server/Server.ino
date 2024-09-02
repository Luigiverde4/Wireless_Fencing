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
  WiFiUDP udp;
  unsigned int port;
  char incomingPacket[255];
  /*
        Paquetes (pck)
  perdidos -> Nº pck perdidos
  primer_id -> ID del 1er pck que llega
  id -> ID del paquete recibido
  cont -> Contador de los pck recibidos
  ultima_medicion_V0 -> Ultimo valor registrado del voltaje del florete V0
  ultima_medicion_V1 -> Ultimo valor registrado del voltaje de la chaquetilla V1

  primera_vez -> T/F de si es la primera vez que se conecta
          Tiempo
  primer_tiempo -> Momento en el que llega el 1er pck
  tienpo -> Momento en el que se recibe el pck 
  diff_tiempo -> Diferencia de tiempo
  */
  unsigned long perdidos;
  unsigned long primer_id;
  unsigned long id;
  unsigned long cont;

  unsigned int ultima_medicion_V0;
  unsigned int ultima_medicion_V1;
  bool primera_vez;

  unsigned long primer_tiempo;
  unsigned long tiempo;
  unsigned long diff_tiempo;

  // Constructor
  UDPConnection(unsigned int p) : 
  port(p),
  perdidos(0), 
  primer_id(0), 
  primer_tiempo(0), 
  tiempo(0),
  id(1), 
  cont(1), 
  ultima_medicion_V0(0),
  ultima_medicion_V1(0),
  primera_vez(1) {}
};

// Crear dos instancias de UDPConnection
UDPConnection connection1(4210);
UDPConnection connection2(4211);

// Matrices de numeros
const byte numeros[16][8] = {
    // 0
    { 
    0b00111000, 
    0b01000100, 
    0b01000100, 
    0b01000100, 
    0b01000100, 
    0b01000100, 
    0b01000100, 
    0b00111000 },

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
    0b01000000,
    0b01111100, 
    0b01000010, 
    0b01000010, 
    0b00111100, 
     },

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
    0b00111100 },
    //  10
    { 
    0b10001110, 
    0b10010001, 
    0b10010001, 
    0b10010001, 
    0b10010001, 
    0b10010001, 
    0b10010001, 
    0b10001110},
    //  11
    { 
    0b00100010,
    0b01100110,
    0b00100010, 
    0b00100010, 
    0b00100010, 
    0b00100010, 
    0b00100010, 
    0b01110111 },
    //  12
    { 
    0b01001110,
    0b11010001,
    0b01000001, 
    0b01001110, 
    0b01010000, 
    0b01010000, 
    0b01010000, 
    0b11101110},
    //  13
    { 
    0b01001110,
    0b11010001,
    0b01000001, 
    0b01001110, 
    0b01000001, 
    0b01000001, 
    0b01001001, 
    0b11101110},
    //  14
    { 
    0b01000010,
    0b11000110,
    0b01001010, 
    0b01010010, 
    0b01011111, 
    0b01000010, 
    0b01000010, 
    0b11100010},

    // 15
    { 
    0b01001111, 
    0b11001000, 
    0b01001000, 
    0b01001110, 
    0b01000001, 
    0b01000001, 
    0b01001001, 
    0b11100110 },

};

const byte numeros_chiquitos[10][4] = {
    // Número 0
    {
      0b01100000, 
      0b10010000, 
      0b10010000, 
      0b01100000
    },
    // Número 1
    {
        0b00100000,
        0b01100000,
        0b00100000,
        0b01110000,
    },
    // Número 2
    {
        0b11100000,
        0b00110000,
        0b01000000,
        0b11110000
    },
    // Número 3
    {
        0b11100000,
        0b00110000,
        0b00110000,
        0b11100000
    },
    // Número 4
    {
        0b10100000,
        0b10100000,
        0b11110000,
        0b00100000
    },
    // Número 5
    {
        0b01110000,
        0b01000000,
        0b00110000,
        0b01110000
    },
    // Número 6
    {
        0b1110000,
        0b1000000,
        0b1110000,
        0b1110000
    },
    // Número 7
    {
        0b11110000,
        0b00100000,
        0b01000000,
        0b01000000,
    },
    // Número 8
    {
        0b01100000,
        0b11010000,
        0b10110000,
        0b01100000
    },
    // Número 9
    {
        0b01110000,
        0b01110000,
        0b00010000,
        0b01110000
    }
};

void setup() {
  Serial.begin(115200);
  Serial.println("SERVIDOR UDP");

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
    // Contadores para comparar si ha llegado paquetes despues
    unsigned long cont1 = connection1.cont;
    unsigned long cont2 = connection2.cont;

    // Si recibimos algun paquete de cualqueira de los dos
    if (recibir_paquete(connection1) || recibir_paquete(connection2)) {
        manejaTocados(connection1, cont1, connection2);
        manejaTocados(connection2, cont2, connection1);
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
      return 1; // Hemos recibido paquete
    }
  }
  return 0; // No hemos recibido un paquete
}

// Funcion para los tocados
void manejaTocados(UDPConnection &principal, unsigned long contaPre, UDPConnection &secundario) {
    // Ha sido el paquete recibido de esta conexion?
    if (contaPre != principal.cont) {
        tratar_paquete(principal);
        if (detectar_tocado(principal)) {
            // Hay tocado en esta conexion
            // Blanco o valido?
            tocadoValido(principal,secundario);

            // Unico o doble?
            manejaDobles(principal,secundario);
        }
    }
    // NO es esta conexion la que nos ha llegado el paquete de
}


// Funcion para asignar valores del paquete y detectar si es la primera vez
void tratar_paquete(UDPConnection conn){
/*
    &UDPConnection conn: Objeto de comunicacion UDP
*/
    // Guardamos la id
    String idStr = obtener_dato(conn.incomingPacket, "ID"); // ID
    conn.id = idStr.toInt();

    // Vemos si es la primera vez que se conecta
    if (conn.primera_vez) {
        // Asignamos tiempos
        conn.primer_tiempo = conn.tiempo;
        conn.diff_tiempo = conn.primer_tiempo - millis();
        // Asignamos IDs
        conn.cont = conn.id;
        conn.primer_id = conn.id;

        conn.primera_vez = 0;
        // Marcamos que ya no es la primera vez
    }

    // Analisis de perdidas
    analizar_perdidas(conn,idStr); 
}
// Funcion para analizar un paquete específico
void analizar_perdidas(UDPConnection &conn, String idStr) {
/*
    &conn: Objeto de comunicacion UDP
    idStr: String de la id del conn
      
      Analiza la conexion y las perdidas
*/
  if (idStr.length() > 0) { // Si ha encontrado el ID
    if (conn.id > conn.cont) { // Si hemos perdido paquetes
      conn.perdidos += (conn.id - conn.cont); // Calculamos los paquetes perdidos
    } 
    conn.cont = conn.id; // Actualizamos el contador al ID del ultimo paquete recibido
  }

  // Porcentaje de paquetes perdidos
  float porcentaje_perdidos = (float)conn.perdidos / (conn.cont - conn.primer_id) * 100.0;
  // Tiempo transcurrido en segundos
  float tiempo_transcurrido = (conn.tiempo - conn.primer_tiempo) / 1000.0;
  // Paquetes Por Segundo (PPS)
  float pps = (conn.cont - conn.primer_id) / tiempo_transcurrido;

  Serial.printf("Puerto %d - Paquetes perdidos: %ld, TOTAL: %ld, PPS: %.2f, Porcentaje perdidos: %.2f%%\n", 
                conn.port, conn.perdidos, conn.cont, pps, porcentaje_perdidos);
}

// Funcion para detectar si esta habiendo un tocado
bool detectar_tocado(UDPConnection &conn){
/*
    UDPConnection &conn: Comunicacion de la que analizar el voltaje
      Devolver true si esta tocando y false si no
*/
    // Analizar voltaje
    String voltajeStr = obtener_dato(conn.incomingPacket, "V0"); // Voltaje florete
    int voltaje_actual = voltajeStr.toInt();
    conn.ultima_medicion_V0 = voltaje_actual;
            
    // Si el voltaje actual es 0, el boton esta presionado y esta abierto el circuito -> V = 0
    if (voltaje_actual == 0) {
        return 1;
    } else { // El boton esta abierto, no esta habiendo tocado
        return 0;
    }
}

// Funcion para detectar si un tocado es valido
bool tocadoValido(UDPConnection &principal, UDPConnection &secundario){
/*
    principal: Conexion UDP del que esta tocando
    secundario Conexion UDP del que esta siendo tocado

    **TIENE QUE ESTAR PASANDO AL MISMO TIEMPO***
*/
    // Si el florete de uno da tocado y el otro recibe por tierra el tocado
    if (principal.ultima_medicion_V0 == 0 && secundario.ultima_medicion_V1 != 0){
        // TOCADO VALIDO
        return 1;
    }else{
        // TOCADO NO VALIDO
        return 0;
    }
    // Si esta el V0J1 y el V1J2 -> Valido
    // Si esta el V0J1 y no e V1J2 -> No Valido
}

// Funcion para los tocados dobles
void manejaDobles(UDPConnection &principal, UDPConnection &secundario) {
    // Durante el hueco de 300ms
    while (millis() - principal.tiempo <= 300) {
        if (recibir_paquete(secundario)) { // Escuchamos a ver si llegan paquetes del secundario
            // LLega paquete del contrario
            tratar_paquete(secundario);
            if (detectar_tocado(secundario)) { // Usamos el voltaje del florete como detector de tocado
                // Hay doble tocado
                // Blanco o valido?
                tocadoValido(secundario,principal); // Hay que pasarlo del reves ya que estamos detectando al reves
                return; // Salir de la funcion
            }
        }
    }

    // No hay doble tocado -  solo tocado principal
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
  
    // Encuentra la posicion en la cadena packetStr donde comienza clave_a_encontrar.
    int IndiceInicial = packetStr.indexOf(clave_a_encontrar);
  
    if (IndiceInicial != -1) { // Si clave_a_encontrar es encontrada
        // Ajusta IndiceInicial para que apunte al comienzo del valor asociado a la clave.
        IndiceInicial += clave_a_encontrar.length();
      
        // Encuentra la posicion del siguiente salto de línea
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





// FUNCIONES MAQUINA
void ponerNumero(byte numero, byte pos_x, bool chiquitos = false, bool despl = false) {
    /*
        byte numero: numero a escribir en la matriz 
        byte pos_x: posición en donde se escribe el numero
        bool chiquitos: si es true, escribe numeros pequeños; si es false, escribe numeros grandes
        bool despl: true - numero chiquito de abajo; false - no abajo
    */
    Serial.print("Numero: "); Serial.println(numero);
    //Serial.print("Pos_x: "); Serial.println(pos_x);
    
    // VARIABLES VARIAS
    int offset = despl ? 4 : 0; // Offset para los numeros pequeños de abajo
    const int limiteColumnas = chiquitos ? 4 : 8; // Limite de columnas 
    const int limiteFilas = 8; // Siempre 8 filas

    // Escoger matriz correcta
    const byte (*matriz)[limiteColumnas] = chiquitos ? (const byte (*)[limiteColumnas])numeros_chiquitos : (const byte (*)[limiteColumnas])numeros;

    // Dibujar el numero en la matriz a partir de pos_x
    for (int columna = 0 + offset; columna < limiteColumnas + offset; columna++) { // Recorremos columnas
        byte valor = matriz[numero][columna - offset]; // Obtener valor de la columna actual
        valor = (chiquitos && despl) ? (valor >>4) : valor; // Queremos coger solo los 4 bits con info

        Serial.print(valor, BIN); // Mostrar valor en formato binario para debug
        
        // Poner el valor en la matriz
        mx.setRow(pos_x, columna, valor);
    }
    Serial.println(); // Nueva línea después de dibujar el numero
}

void ponerTiempo(byte minutos, byte segundos, bool min_cambio = false) {
/*
    byte minutos: Cantidad de minutos restantes
    byte segundos: Cantidad de segundos restantes
    bool min_cambio: Si ha cambiado el minuto

        Pone el tiempo del cronometro en la matriz
*/
    Serial.print("Segundos: ");Serial.println(segundos);

    // Mitad superior (decenas de segundos)
    int decenas_segundos = segundos / 10; // Decenas de segundos
    ponerNumero(decenas_segundos,1,true,false);

    
    // Mitad inferior (unidades de segundos)
    int unidades_segundos = segundos % 10; // Unidades de segundos
    ponerNumero(unidades_segundos,1,true,true);
    
    // Mostrar minutos en la matriz, comenzando en la posición 15
    if (min_cambio){ponerNumero(minutos, 2);}
}

void vaciarMatriz() {
/*
    Asigna los valores de las filas de cada columna a 0
*/
    for (int x = 0; x < 32; x++) {
        for (int y = 0; y<8;y++){
            mx.setPoint(y, x, false); 
        }
    }
}

// Manipular puntos
void ponerPuntos(byte &puntaje1, byte &puntaje2) {
/*
    byte &puntaje1: Valor del puntaje global para el tirador 1
    byte &puntaje2: Valor del puntaje global para el tirador 2  
        
        Pone los puntos a los tiradores y los limita entre 0 y 15 
*/
    puntaje1 = constrain(puntaje1, 0, 15);
    puntaje2 = constrain(puntaje2, 0, 15);
    // Copiar los puntos en la matriz
    ponerNumero(puntaje1, 3);    // Puntaje jugador 1, posición 24
    ponerNumero(puntaje2, 0);     // Puntaje jugador 2, posición 0
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
    unsigned long tiempo_transcurrido = millis();
    const static long intervalo = 1000;
    bool min_cambio;

    if (segundos == 0 && minutos != 0){ // No es el ultimo minuto y se ha acabado el actual
        segundos = 60;
        minutos--;
        min_cambio = true; // Aqui hay un fallo y nunca sale el 59!!!

    }else if(segundos == 0 && minutos == 0){ // Se ha acabado el tiempo
        segundos = 0;
        minutos = 0;
        return;
    }

    if (tiempo_transcurrido - tiempo_previo >= intervalo) { // Ha pasado 1seg ?
        tiempo_previo = tiempo_transcurrido;
        segundos--;
        min_cambio = false;
    }
    ponerTiempo(minutos,segundos,min_cambio);
}

void resetTiempo(){
/*
    Resetea el tiempo a 3min y 0seg
*/
    minutos = 3;
    segundos = 0;
}
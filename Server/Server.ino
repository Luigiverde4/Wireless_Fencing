#include <WiFi.h>
#include <maquina.h>

// Maquina
MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);
byte puntaje1 = 0;
byte puntaje2 = 0;
byte minutos = 3;
byte segundos = 0;
bool tiempo_maquina = false;
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

  // Constructor
  UDPConnection(unsigned int p) : 
  port(p),
  perdidos(0), 
  primer_id(0), 
  primer_tiempo(0), 
  tiempo(0),
  id(1), 
  cont(1), 
  ultima_medicion_V0(-1),
  ultima_medicion_V1(-1),
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
    // Numero 0
    {
      0b01100000, 
      0b10010000, 
      0b10010000, 
      0b01100000
    },
    // Numero 1
    {
        0b00100000,
        0b01100000,
        0b00100000,
        0b01110000,
    },
    // Numero 2
    {
        0b11100000,
        0b00110000,
        0b01000000,
        0b11110000
    },
    // Numero 3
    {
        0b11100000,
        0b00110000,
        0b00110000,
        0b11100000
    },
    // Numero 4
    {
        0b10100000,
        0b10100000,
        0b11110000,
        0b00100000
    },
    // Numero 5
    {
        0b01110000,
        0b01000000,
        0b00110000,
        0b01110000
    },
    // Numero 6
    {
        0b1110000,
        0b1000000,
        0b1110000,
        0b1110000
    },
    // Numero 7
    {
        0b11110000,
        0b00100000,
        0b01000000,
        0b01000000,
    },
    // Numero 8
    {
        0b01100000,
        0b11010000,
        0b10110000,
        0b01100000
    },
    // Numero 9
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
//   mx.begin();
//   mx.control(MD_MAX72XX::INTENSITY, MAX_INTENSITY / 2);
//   mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
//   mx.clear();  

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

//Flags de si han llegados paquetes
bool actualizado1;
bool actualizado2;
void loop() {
    // Actualizamos las conexiones de si han llegados paquetes 
    actualizado1 = recibir_paquete(connection1);
    actualizado2 = recibir_paquete(connection2);

    // Procesar los paquetes si hay actualizaciones
    if (actualizado1) { 
        manejaTocados(connection1, connection2);
    }
    if (actualizado2) {
        manejaTocados(connection2, connection1);
    }

    // switch(){case} Switch case para el infrarrojo
    if (tiempo_maquina){
        cuentaAtras();
    }
}

// Funcion para recibir paquetes de una conexion UDP especifica
bool recibir_paquete(UDPConnection &conn) {
  //Serial.println("Recibir paquete");
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
void manejaTocados(UDPConnection &principal, UDPConnection &secundario) {
    Serial.printf("Maneja Tocados: %u\n", principal.port);

    tratar_paquete(principal,secundario);
    if (detectar_tocado(principal)) {
        // Hay tocado en esta conexion
        // Unico o doble?
        manejaDobles(principal,secundario);
        // Ya hemos cogido mas paquetes 
        // Blanco o valido?
        tocadoValido(principal,secundario);
    }

    // NO es esta conexion la que nos ha llegado el paquete de
}


// Funcion para asignar valores del paquete y detectar si es la primera vez
void tratar_paquete(UDPConnection &conn, UDPConnection &secundario){
    Serial.println("Tratar paquete");
/*
    &UDPConnection conn: Objeto de comunicacion UDP
*/

    // A FUTURO MOVER ESTO A RECIBIR PAQUETE Y DEJAR SOLO EL IF
    // Buffers
    char idStr[11];  // Suponiendo que el ID no superara los 11 caracteres + '\0'
    // Voltaje0
    char voltajeStr0[11];  // Suponiendo que el voltaje no supera los 10 caracteres + \0
    // Voltaje1
    char voltajeStr1[11];  // Suponiendo que el voltaje no supera los 10 caracteres + \0

    // Obtener_datos
    obtener_dato(conn.incomingPacket, "V0:", voltajeStr0, sizeof(voltajeStr0));
    obtener_dato(conn.incomingPacket, "ID:", idStr, sizeof(idStr));
    obtener_dato(secundario.incomingPacket, "V1:", voltajeStr1, sizeof(voltajeStr1));


    // V0
    int voltaje_actual0 = atoi(voltajeStr0);
    conn.ultima_medicion_V0 = voltaje_actual0;

    // V1
    int voltaje_actual1 = atoi(voltajeStr1);
    secundario.ultima_medicion_V1 = voltaje_actual1;

    // ID
    conn.id = atoi(idStr);

    // Vemos si es la primera vez que se conecta
    if (conn.primera_vez) {
        // Asignamos tiempos
        conn.primer_tiempo = conn.tiempo;
        // Asignamos IDs
        conn.cont = conn.id;
        conn.primer_id = conn.id;

        conn.primera_vez = false;
        // Marcamos que ya no es la primera vez
    }

    // Analisis de perdidas
    //analizar_perdidas(conn,idStr); 
}

// Funcion para analizar un paquete especifico
void analizar_perdidas(UDPConnection &conn, String idStr) {
        Serial.println("Analizar perdidas");

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
    Serial.println("Detectar Tocado");
/*
    UDPConnection &conn: Comunicacion de la que analizar el voltaje
      Devolver true si esta tocando y false si no
*/

    // Si el voltaje actual es 0, el boton esta presionado y esta abierto el circuito -> V = 0
    if (conn.ultima_medicion_V0 == 0) {
        return 1;
    } else { // El boton esta abierto, no esta habiendo tocado
        return 0;
    }
}

// Funcion para detectar si un tocado es valido
bool tocadoValido(UDPConnection &principal, UDPConnection &secundario){
    Serial.println("Tocado Valido");

/*
    principal: Conexion UDP del que esta tocando
    secundario Conexion UDP del que esta siendo tocado

    **TIENE QUE ESTAR PASANDO AL MISMO TIEMPO***
*/

    Serial.printf("Principal: %u V0: %u Secundario: %u V1: %u\n",principal.port,principal.ultima_medicion_V0,secundario.port,secundario.ultima_medicion_V1);
    // Si el florete de uno da tocado y el otro recibe por tierra el tocado
    if (principal.ultima_medicion_V0 == 0 && secundario.ultima_medicion_V1 != 0){
        // TOCADO VALIDO
        Serial.printf("TOCADO VALIDO de %u\n", principal.port);
        Serial.printf("Diferencia de tiempo: %u\n", principal.tiempo - secundario.tiempo);
        return 1;
    }else{
        // TOCADO NO VALIDO
        Serial.printf("TOCADO NO VALIDO de %u\n", principal.port);
        Serial.printf("Diferencia de tiempo: %u\n", principal.tiempo - secundario.tiempo);

        return 0;
    }
    // Si esta el V0J1 y el V1J2 -> Valido
    // Si esta el V0J1 y no e V1J2 -> No Valido
}



// Funcion para manejar tocados dobles
void manejaDobles(UDPConnection &principal, UDPConnection &secundario) {
    Serial.println("Maneja Dobles");

    // Coger la flag correcta
    bool flag_actualizado = (principal.port == connection1.port) ? actualizado1 : actualizado2;

    // Verificar si necesitamos paquetes nuevos o tenemos paquetes recientes
    if (flag_actualizado) {
        unsigned long tiempo_diff = (principal.tiempo > secundario.tiempo)  // Vemos cual es mas grande
                                    ? principal.tiempo - secundario.tiempo 
                                    : secundario.tiempo - principal.tiempo;
        if (tiempo_diff <= 300 && secundario.tiempo >= principal.tiempo) { // && secundario.tiempo >= principal.tiempo (para asegurarnos que coge uno igual o despues del principal)
            // Tenemos paquete suficientemente reciente
            tratar_paquete(secundario,principal);
            
            if (detectar_tocado(secundario)) { // Usamos el voltaje del florete como detector de estar haciendo tocado
                Serial.printf("Tocado YA detectado de %u\n",secundario.port);
                // Hay doble tocado
                // Blanco o valido?
                tocadoValido(secundario, principal); // Hay que pasarlo del reves ya que estamos detectando al reves
                //Serial.printf("Diferencia entre %u y %u: %u",principal.port,secundario.port,tiempo_diff)
                return; // Salir de la funcion
            }
            // No hay doble tocado - solo tocado principal
        } else {
            // Necesitamos paquetes mas recientes aunque tengamos ya
            pedir_paquetes(principal, secundario);
        }
    } else {
        // Necesitamos paquetes
        pedir_paquetes(principal, secundario);
    }
    //delay(4000);
}

void pedir_paquetes(UDPConnection &principal, UDPConnection &secundario){
    Serial.println("Pedir Paquetes");
    while (millis() - principal.tiempo <= 300){ // Durante los 300ms de ventana escuchar paquetes
        if (recibir_paquete(secundario)) { // Escuchamos a ver si llegan paquetes del secundario
            // LLega paquete del contrario
            tratar_paquete(secundario,principal);
            if (detectar_tocado(secundario)) { // Usamos el voltaje del florete como detector de tocado
                Serial.printf("Tocado pedido  de %u\n",secundario.port);
                // Hay doble tocado
                // Blanco o valido?
                tocadoValido(secundario,principal); // Hay que pasarlo del reves ya que estamos detectando al reves
                return; // Salir de la funcion
            }
        }
    }
    // No hay doble tocado -  solo tocado principal
}


// Funcion para obtener un dato especifico del paquete usando char[]
void obtener_dato(const char* packet, const char* clave, char* valor, size_t maxLen) {
    //Serial.println("Obtener dato");

/*
      
      packet: Contenido del paquete como cadena de caracteres
      clave:  Dato que queremos sacar
      valor: Es el valor que devolvemos de la funcion
      maxLen: Longitud maxima del valor que vamos a devolver

      Saca datos del mensaje enviado por UDP con char[], si no lo encuentra el valor sera \0
*/
     // Busca en el paquete la clave y apunta al principio del valor, si no lo encuentra es NULL
    char* inicio = strstr(packet, clave);
    if (inicio == NULL) { // Si NO encuentra la clave en el paquete
        valor[0] = '\0'; // Devolvemos una cadena vacia
    } else {
        inicio += strlen(clave);
        char* fin = strstr(inicio, "\n"); // Buscamos el primer salto de linea desde el incio (cogemos todo el valor)
        
        if (fin == NULL) {  // Si no lo encuentra, es que es el ultimo y se acaba el paquete
            fin = inicio + strlen(inicio); // Coge toda la cadena
        }

        size_t len = fin - inicio; // Longitud del valor
        if (len >= maxLen){len = maxLen - 1;} // Si es mas largo que el maximos -1 (para meter \0), lo forzamos a ese tamaño y dejamos hueco para \0
        
        strncpy(valor, inicio, len); // Copiamos en valor desde inicio una distancia len
        valor[len] = '\0';          // Ponemos el 0 al final para cerrar la cadena
    }
}




// FUNCIONES MAQUINA
void ponerNumero(byte numero, byte pos_x, bool chiquitos = false, bool despl = false) {
    /*
        byte numero: numero a escribir en la matriz 
        byte pos_x: posicion en donde se escribe el numero
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
    Serial.println(); // Nueva linea despues de dibujar el numero
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
    
    // Mostrar minutos en la matriz, comenzando en la posicion 15
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
    ponerNumero(puntaje1, 3);    // Puntaje jugador 1, posicion 24
    ponerNumero(puntaje2, 0);     // Puntaje jugador 2, posicion 0
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
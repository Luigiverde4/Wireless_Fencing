#ifndef MAQUINA_H
#define MAQUINA_H

#include <MD_MAX72xx.h>
#include <SPI.h>

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define CLK_PIN   13  // SCK
#define DATA_PIN  11  // MOSI
#define CS_PIN    10  // SS

MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);
// Valores iniciales y globales del marcador
byte puntaje1 = 0;   // Puntaje J1
byte puntaje2 = 0;   // Puntaje J2
byte minutos = 3;    // Minutos
byte segundos = 0;   // Segundos


// IR - Infrarrojo
#define IR_USE_AVR_TIMER5
#include <IRremote.hpp>
#define IR_RECEIVE_PIN 12

// Matriz
byte matriz_led[32]; // 8 bits (valores filas) , 32 columnas. Ahorramos bytes

// Cada byte representa una columna en una matriz de 8x8 LEDs.
const byte numeros[10][8] = {
    // numero 0
    { 
    0b00111100, 
    0b01000100, 
    0b01000100, 
    0b01000100, 
    0b01000100, 
    0b01000100, 
    0b01000100, 
    0b00111100 },

    // numero 1
    { 
    0b00010000,
    0b00110000,
    0b01010000, 
    0b00010000, 
    0b00010000, 
    0b00010000, 
    0b00010000, 
    0b00111100 },

    // numero 2
    { 
    0b00111100, 
    0b01000010, 
    0b00000010, 
    0b00011100, 
    0b00100000, 
    0b01000000, 
    0b01000000, 
    0b01111110 },

    // numero 3
    { 
    0b00111100, 
    0b01000010, 
    0b00000010, 
    0b00111100, 
    0b00000010, 
    0b00000010, 
    0b01000010, 
    0b00111100 },

    // numero 4
    { 
    0b00001100, 
    0b00010100, 
    0b00100100, 
    0b01000100, 
    0b01111111, 
    0b00000100, 
    0b00000100, 
    0b00000100 },

    // numero 5
    { 
    0b01111110, 
    0b01000000, 
    0b01000000, 
    0b01111100, 
    0b00000010, 
    0b00000010, 
    0b01000010, 
    0b00111100 },

    // numero 6
    { 
    0b00111100, 
    0b01000000, 
    0b01000000, 
    0b01111100, 
    0b01000010, 
    0b01000010, 
    0b00111100, 
    0b00000000 },

    // numero 7
    { 
    0b01111110, 
    0b00000010, 
    0b00000100, 
    0b00001000, 
    0b00010000, 
    0b00100000, 
    0b01000000, 
    0b01000000 },

    // numero 8
    { 
    0b00111100, 
    0b01000010,
    0b01000010,
    0b00111100, 
    0b01000010, 
    0b01000010, 
    0b01000010, 
    0b00111100 },

    // numero 9
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
            int valor = (numeros[numero][columna] >> fila) & 1;
            // Escribimos en memoria
            matriz_led[pos_x + columna][fila] = valor;
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
            matriz_led[8 + columna][fila] = numeros_chiquitos[decenas_segundos][fila][columna]; // Empezamos desde la columna 18, lo mas arriba posible , fila 0
        }
    }
    
    // Mitad inferior
    int unidades_segundos = segundos % 10; // Segundos
    for (int fila = 0; fila < 4; fila++) { // Recorremos las 4 filas
        for (int columna = 0; columna < 4; columna++) { // Recorremos las 4 columnas
            // Copiar bits de numeros_chiquitos al arreglo matrix_led para la mitad inferior
            matriz_led[12 + columna][fila + 4] = numeros_chiquitos[unidades_segundos][fila][columna]; // Empezamos desde la columna 20, fila 4
        }
    }
    */
}
void vaciarMatriz() {
/*
    Asigna los valores de las filas de cada columna a 0
*/
    for (int x = 0; x < 32; x++) {
        matriz_led[x] = 0;
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
#ifndef MAQUINA_H
#define MAQUINA_H

#include <MD_MAX72xx.h>
#include <SPI.h>
// #include <IRremote.hpp> da error - no funciona con el core del ESP32 3.x

// Definiciones de hardware
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define CLK_PIN   13  // SCK
#define DATA_PIN  11  // MOSI
#define CS_PIN    10  // SS
#define IR_RECEIVE_PIN 12
#define IR_USE_AVR_TIMER5

/*
MOSI: 23
MISO: 19
SCK: 18
SS: 5
*/
// Funciones para manipulación de la matriz y puntuaciones
void ponerNumero(byte numero, byte pos_x, bool chiquitos, bool despl);
void ponerTiempo(byte minutos, byte segundos, bool min_cambio);
void ponerTiempo(int minutos, int segundos);
void vaciarMatriz();
void ponerPuntos(byte &puntaje1, byte &puntaje2);
void resetPuntos();
void cuentaAtras();
void resetTiempo();

// Variables globales
extern byte puntaje1;
extern byte puntaje2;
extern byte minutos;
extern byte segundos;

#endif // MAQUINA_H
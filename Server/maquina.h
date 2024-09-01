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
void ponerNumero(int numero, int pos_x);
void ponerPuntos(int puntaje1, int puntaje2);
void ponerTiempo(int minutos, int segundos);
void vaciarMatriz();
void manipularPunto(int jugador, int cambio);
void resetPuntos();
void cuentaAtras();
void resetTiempo();

// Variables globales
extern byte puntaje1;
extern byte puntaje2;
extern byte minutos;
extern byte segundos;

#endif // MAQUINA_H
/*
 * Ce fichier défini tout les paramètres du système de déploiement des parachutes de la fusée.
 */

#ifndef _configCircuitDeploiement_h
#define _configCircuitDeploiement_h

#include <SPI.h>
#include <Wire.h>
#include "Adafruit_BMP085.h"
#include "SD.h"


//-----------------------------------------------------------------------------------
//  Hardware pinout

// BMP180 -> Arduino Nano
// -      -> GND
// +      -> VCC (3.3V)
// IO     -> Non Connecté
#define IO_BMP180_DA    A4           // I2C data, ne peut pas etre changé
#define IO_BMP180_CL    A5           // I2C clock. ne peut pas etre changé

// SD BOARD -> Arduino Nano
// CD       -> Non Connecté
// GND      -> GND
// VCC      -> VCC (3.3V)
#define IO_SD_DO     12              // Data out, ne peut pas etre changé
#define IO_SD_SCK    13              // Slave clock, ne peut pas etre changé
#define IO_SD_DI     11              // Data in, ne peut pas etre changé
#define IO_SD_CS     10              // Chip selec, ne peut pas etre changé, car elle dépend du microcontroleur

// Buzzer
#define IO_BUZZER_OUT   3         // Sortie du signal sonore pour le buzzer

// Parachutes
#define IO_DROGUE_CTRL      5 // Sortie du signal de déploiement du drogue
#define IO_MAIN_CTRL        6 // Sortie du signal de déploiement du main
#define IO_DROGUE_STATE     7 // Entrée vérifiant la continuité de l'alumette du drogue
#define IO_MAIN_STATE       8 // Entrée vérifiant la continuité de l'alumette du main

//-----------------------------------------------------------------------------------
//  Altimeter

#define ALTIMETER_INVALID_ALTITUDE_TOLERANCE    1000.0

//-----------------------------------------------------------------------------------
//  Log unit

// Paramètres
#define LOG_UNIT_SERIAL_BAUDRATE  115200
#define LOG_UNIT_FILE_NAME        "alt"
#define LOG_UNIT_MAX_NB_OF_FILES  99
#define LOG_UNIT_FILE_EXT         ".csv"

// Messages d'évènements
#define MESSAGE_BURNOUT_STARTED     "burnout started"
#define MESSAGE_BURNOUT_FINISHED    "burnout finished"
#define MESSAGE_DROGUE_OUT          "drogue out"
#define MESSAGE_DROGUE_ALREADY_OUT  "drogue already out"
#define MESSAGE_MAIN_OUT            "main out"
#define MESSAGE_MAIN_ALREADY_OUT    "main already out"
#define MESSAGE_FLIGHT_FINISHED     "flight finished"
#define MESSAGE_INVALID_ALTITUDE    "invalid altitude"

// Format du fichier log
#define ID_LOG_MESSAGE    0   //chiffre qui va avoir au début de chaque ligne du header qui est un message d'information
#define ID_LOG_DATA       1   //chiffre qui va avoir au début de chaque ligne qui est du data
#define ID_LOG_EVENT      2   //chiffre qui va avoir au début de chaque ligne du header qui est un evenement


//-----------------------------------------------------------------------------------
//  Buzzer

// Paramètres du signal sonore
#define BUZZER_TIME_BETWEEN_SEQUENCES 4000
#define BUZZER_CYCLE_DURATION         500


//-----------------------------------------------------------------------------------
//  Matches

// Id des parachutes
#define ID_PARACHUTE_DROGUE  0
#define ID_PARACHUTE_MAIN    1

// Tag de l'état des parachutes
#define TAG_PARACHUTE_NULL        0
#define TAG_PARACHUTE_DROGUE_ONLY 1
#define TAG_PARACHUTE_MAIN_ONLY   2
#define TAG_PARACHUTE_BOTH        3

//----------------------------------------------------------------------------
// Low pass filter's parameters
/*
 * The filter used is an elliptical low pass filter. The equation was computed
 * using Matlab's Ellip(() function)
 * 
 * The filter's parameters are:
 *     N   = 3;        filter order
 *     Wp  = 0.1;      cutoff frequency
 *     Rp  = 0.05;     Peak-to-peak ripple      
 *     Rst = 40;       Stopband attenuation
 * 
 * The filter's equation is:
 *    A0*y[n] = B0*x[n] + B1*x[n-1] + B2*x[n-2] + B3*x[n-3] - A1*y[n-1] - A2*y[n-2] - A3*y[n-3]
 */

#define ALTITUDE_FILTER_ORDER   3
#define ALTITUDE_ARRAY_SIZE     (ALTITUDE_FILTER_ORDER+1)

const float A[ALTITUDE_ARRAY_SIZE] = {
    1,                    // A0
    -2.242002473393440,   // A1
    1.789446106565067,    // A2
    -0.495145905738350    // A3
};

const float B[ALTITUDE_ARRAY_SIZE] = {
    0.019249185590260,    // B0
    0.006899678126378,    // B1
    0.006899678126378,    // B2
    0.019249185590260     // B3
};


//----------------------------------------------------------------------------
//  Paramètres de la boucle principale

// Fréquence d'échantillonage
#define DATA_SAMPLING_PERIOD    100000 // 100000 micro-secondes ou un fréquence de 10 Hz.

//----------------------------------------------------------------------------
// Breakpoint d'altitude (m) et de vitessse (m/ech) ou un echétantillion est 0,1 s
#define BREAKPOINT_SPEED_TO_BURNOUT     3    // 30 m/s - Breakpoint pour passer du launchpad au burnout
#define BREAKPOINT_SPEED_TO_PRE_DROGUE  3    // 30 m/s - Breakpoint pour passer du burnout au predrogue
#define BREAKPOINT_SPEED_TO_IDLE        0.01  // 0,1 m/s - Breakpoint pour passer de la descent à innactif au sol

#define BREAKPOINT_ALTITUDE_TO_BURNOUT  6   // 6 m - Breakpoint pour passer du launchpad au burnout
#define BREAKPOINT_ALTITUDE_TO_DRIFT    460 // 450 m - Breakpoint pour passer de premain à descente

// Breakpoint du nombre d'échantillion d'altitude entre l'apogée et le déploiement du drogue.
#define BREAKPOINT_DELTA_TIME_APOGEE    4  // 0,4s

// Range d'altitude prévu
#define FLIGHT_MINIMAL_ALTITUDE         0.0 // 0 m
#define FLIGHT_MAXIMAL_ALTITUDE         4000.0 // 4000 m ou 13000 pieds

#endif

/*
Ce programme vise à être utilisé avec le système de déploiement des parachutes

Il permet de faire la mesure d'altitude à partir d'un altimètre barométrique 
BMP et de l'enregistrer sur une carte SD, et envoi le signal de déploiement du 
drogue à l'apogée et celui du déploiement du parachute principal lors de la 
descente, lorsque l'altitude atteint un certain seuil. De plus, un buzzer 
signal lorsqu'il y a un continuité dans les allumettes.
*/

// standard libraries includes
#include <SPI.h>
#include <Wire.h>
#include <SD.h>

// project libraries includes
#include "config_deploiement.h"
#include "parachute.h"


#define TEST_MODE 0

enum FlightState {
    FLIGHT_LAUNCHPAD,
    FLIGHT_BURNOUT,
    FLIGHT_PRE_DROGUE,
    FLIGHT_PRE_MAIN,
    FLIGHT_DRIFT,
    FLIGHT_LANDED
};

FlightState current_flight_state = FLIGHT_LAUNCHPAD;


parachute para_drogue;
parachute para_main;


int check_parachutes(parachute* para);


void setup() {
    init_parachute(&para_drogue, IO_DROGUE_CTRL, IO_DROGUE_STATE);
    init_parachute(&para_main, IO_MAIN_CTRL, IO_MAIN_STATE);
    
}

void loop() {
    switch(current_flight_state) {
        case FLIGHT_LAUNCHPAD:
            
            break;
        case FLIGHT_BURNOUT:
            
            break;
        case FLIGHT_PRE_DROGUE:
            
            break;
        case FLIGHT_PRE_MAIN:
            
            break;
        case FLIGHT_DRIFT:
            
            break;
        case FLIGHT_LANDED:
            
            break;
    }
}

int check_parachutes(parachute* p_main, parachute* p_drogue) {
    // get states of the parachutes
    int main_state;
    int drogue_state;
    main_state = check_connection(p_main);
    drogue_state = check_connection(p_drogue);
    // play appropriate buzzer sequence
    // TODO: buzzer sequence in check_parachutes function
    // return the global parachute state
    return main_state + drogue_state;
}

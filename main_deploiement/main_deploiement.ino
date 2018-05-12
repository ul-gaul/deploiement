/*
Ce programme vise à être utilisé avec le système de déploiement des parachutes

Il permet de faire la mesure d'altitude à partir d'un altimètre barométrique 
BMP et de l'enregistrer sur une carte SD, et envoi le signal de déploiement du 
drogue à l'apogée et celui du déploiement du parachute principal lors de la 
descente, lorsque l'altitude atteint un certain seuil. De plus, un buzzer 
signal lorsqu'il y a un continuité dans les allumettes.
*/

// standard libraries includes
#include <Wire.h>

// project libraries includes
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


void setup() {
    
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

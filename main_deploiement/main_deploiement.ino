/*
Ce programme vise à être utilisé avec le système de déploiement des parachutes

Il permet de faire la mesure d'altitude à partir d'un altimètre barométrique BMP et de
l'enregistrer sur une carte SD, et envoi le signal de déploiement du drogue à l'apogée et
celui du déploiement du parachute principal lors de la descente, lorsque l'altitude atteint un
certain seuil. De plus, un buzzer signal lorsqu'il y a un continuité dans les allumettes.
----------------------------------------------------------------------------------------------
 */
 
#include <SPI.h>
#include <Wire.h>
#include <TimerOne.h>
#include "rocket.h"

Rocket rocket;

bool altitudeUpToDate;
byte flightPlanStep;
byte apogeeTimeCounter;


void setup() {
    // TODO: Implémenter un gestion en cas de reset du Arduino Nano pendant le vol
    altitudeUpToDate = false;
    flightPlanStep = FLIGHT_STEP_LAUNCHPAD;
    apogeeTimeCounter = 0;
    
    rocket.initHardware();
    Timer1.initialize(DATA_SAMPLING_PERIOD);
    Timer1.attachInterrupt(requireAltitudeUpdate);
}

void loop() {
    bool validAltitude;   
    if(altitudeUpToDate == false) {
        validAltitude = rocket.updateAltitude();
        if(validAltitude) {
            rocket.logData();
            followFlightPlan();
            altitudeUpToDate = true;
        }
        else {
            rocket.logEvent(MESSAGE_INVALID_ALTITUDE);
        }
    }
}

void requireAltitudeUpdate() { 
// Fonction appellée par le timer interrupt. Marque les valeurs comme étant non à jour.
  altitudeUpToDate = false; 
}

void followFlightPlan() {
    switch(flightPlanStep) {
        case FLIGHT_STEP_LAUNCHPAD:
            rocket.verifyParachutes();
            if(rocket.getSpeed() > BREAKPOINT_SPEED_TO_BURNOUT && rocket.getAltitude(0) > BREAKPOINT_ALTITUDE_TO_BURNOUT) {
                rocket.logEvent(MESSAGE_BURNOUT_STARTED);
                flightPlanStep = FLIGHT_STEP_BURNOUT;            
            }
            break;
            
        case FLIGHT_STEP_BURNOUT:
            if(rocket.getSpeed() < BREAKPOINT_SPEED_TO_PRE_DROGUE) {
                rocket.logEvent(MESSAGE_BURNOUT_FINISHED); 
                flightPlanStep = FLIGHT_STEP_PRE_DROGUE;
            }
            break;

        case FLIGHT_STEP_PRE_DROGUE:
            if(countApogeeTime() >= BREAKPOINT_DELTA_TIME_APOGEE) {
                if(rocket.verifyParachutes() == TAG_PARACHUTE_NULL | rocket.verifyParachutes() == TAG_PARACHUTE_MAIN_ONLY) {
                    rocket.logEvent(MESSAGE_DROGUE_ALREADY_OUT);
                }
                rocket.deployParachute(ID_PARACHUTE_DROGUE);
                rocket.logEvent(MESSAGE_DROGUE_OUT);
                flightPlanStep = FLIGHT_STEP_PRE_MAIN;
            }
            break;

        case FLIGHT_STEP_PRE_MAIN:
            if(rocket.getAltitude(0) < BREAKPOINT_ALTITUDE_TO_DRIFT) {
                if(rocket.verifyParachutes() == TAG_PARACHUTE_NULL) {
                    rocket.logEvent(MESSAGE_MAIN_ALREADY_OUT);
                }
                rocket.deployParachute(ID_PARACHUTE_MAIN);
                rocket.logEvent(MESSAGE_MAIN_OUT);
                flightPlanStep = FLIGHT_STEP_DRIFT;
            }
            break;

        case FLIGHT_STEP_DRIFT:
            if(rocket.getSpeed() < BREAKPOINT_SPEED_TO_IDLE) {
                rocket.logEvent(MESSAGE_FLIGHT_FINISHED);
                flightPlanStep = FLIGHT_STEP_IDLE;      
            }
            break;

        case FLIGHT_STEP_IDLE:
            rocket.verifyParachutes();
            break;
    }
}


byte countApogeeTime() {
    if(rocket.getMaxAltitude() == rocket.getAltitude(0)) {
      apogeeTimeCounter = 0;
    }
  
    else {
      apogeeTimeCounter ++;
    }
}


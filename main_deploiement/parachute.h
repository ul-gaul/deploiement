/*
 * Librairie pour interfacer le contrôle des matchs 
 * électroniques des parachutes.
 */

#ifndef PARACHUTE_H
#define PARACHUTE_H

#include <Arduino.h>

typedef struct {
    unsigned int control_pin;
    unsigned int state_pin;
} parachute;

void init_parachute(parachute* para, unsigned int control, unsigned int state);

boolean check_connection(parachute* para);

void deploy_parachute(parachute* para);


#endif

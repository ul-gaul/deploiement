/*
 * Library made to interface the control of the electronic matches controlling
 * the deployment of the parachutes
 */

#ifndef PARACHUTE_H
#define PARACHUTE_H

#include <Arduino.h>

typedef struct {
    unsigned int control_pin;
    unsigned int state_pin;
} parachute;

void init_parachute(parachute* para, unsigned int control, unsigned int state);

int check_connection(parachute* para);

void deploy_parachute(parachute* para);


#endif

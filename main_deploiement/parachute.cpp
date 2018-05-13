#include "parachute.h"

void init_parachute(parachute* para, unsigned int control, unsigned int state) {
    para->control_pin = control;
    para->state_pin = state;
    pinMode(para->control_pin, OUTPUT);
    pinMode(para->state_pin, INPUT);
    digitalWrite(para->control_pin, LOW);
}

unsigned int check_connection(parachute* para) {
    return digitalRead(para->state_pin);
}

void deploy_parachute(parachute* para) {
    digitalWrite(para->control_pin, HIGH);
}

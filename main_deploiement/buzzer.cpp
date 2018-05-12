#include "buzzer.h"


// private functions
void buzzerON(buzzer* buz) {
    digitalWrite(buz->pin, HIGH);
}

void buzzerOFF(buzzer* buz) {
    digitalWrite(buz->pin, LOW);
}

// public functions
void init_buzzer(buzzer* buz, unsigned int p_pin, unsigned int p_sequence_time,
                 unsigned int p_period) {
    // assign buzzer pin
    buz->pin = p_pin;
    // assign sequence time
    p_sequence_time = (p_sequence_time > MAX_SEQUENCE_TIME) ? MAX_SEQUENCE_TIME : p_sequence_time;
    buz->sequence_time = (p_sequence_time < MIN_SEQUENCE_TIME) ? MIN_SEQUENCE_TIME : p_sequence_time;
    // assign time period of buzzing
    p_period = (p_period > MAX_PERIOD) ? MAX_PERIOD : p_period;
    buz->period = (p_period < MIN_PERIOD) ? MIN_PERIOD : p_period;
    // initialize sequence beginning time
    buz->sequence_begin_time = 0;
    // initialize sequence time counter
    buz->counter = 0;
    // initialize current cycle
    buz->current_cycle = 0;
}

void execute_sequence(buzzer* buz, unsigned int cycles) {
    unsigned int timenow = millis();
    buz->counter = timenow - buz->sequence_begin_time;
    // check if this is the start of a new sequence
    if(buz->counter >= buz->sequence_time) {
        buz->sequence_begin_time = timenow;
        buz->counter = 0;
    }
    // check progress in current sequence and activate the buzzer accordingly
    // TODO: algorithm for the buzzing
}

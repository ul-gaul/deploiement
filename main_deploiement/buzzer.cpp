#include "buzzer.h"


// private functions
void buzzerON(buzzer* buz) {
    digitalWrite(buz->pin, HIGH);
}

void buzzerOFF() {
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
    // assign sequence beginning time
    buz->sequence_begin_time = 0;
}

void execute_sequence(buzzer* buz, unsigned int cycles) {
    
}

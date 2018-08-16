/*
 * Simple interface for a buzzer. The buzzer is controlled in a binary fashion
 * by a GPIO. On a high value (3.3V), the buzzer emits a sound. On a low value
 * (0V) the buzzer is silent. To indicate the state of the parachutes, the
 * buzzer will emit a number of short beeps followed by a silence. The number
 * of beeps corresponds to the state of the two parachutes
 * (see main_deploiement.ino for details) and the duration of the silence is
 * given by the sequence period, as the sequence is repeted constantly.
 */

#ifndef BUZZER_H
#define BUZZER_H

#include <Arduino.h>

#define MIN_SEQUENCE_TIME 5000
#define MAX_SEQUENCE_TIME 10000
#define MIN_PERIOD 500
#define MAX_PERIOD 1500


typedef struct {
	unsigned int pin;
	unsigned int sequence_period;
	unsigned int cycle_period;
	unsigned int sequence_begin_time;
	unsigned int time_counter;
} buzzer;

void buzzerON(buzzer* buz);

void buzzerOFF(buzzer* buz);

void init_buzzer(buzzer* buz, unsigned int p_pin, unsigned int p_sequence_time,
	unsigned int p_period);

void execute_sequence(buzzer* buz, unsigned int cycles);

#endif

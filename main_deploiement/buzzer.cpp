#include "buzzer.h"


// private functions
void buzzerON(buzzer* buz) {
	analogWrite(buz->pin, 255);
}

void buzzerOFF(buzzer* buz) {
	analogWrite(buz->pin, 0);
}

// public functions
void init_buzzer(buzzer* buz, unsigned int p_pin, unsigned int p_sequence_period,
		unsigned int p_cycle_period) {
	// assign buzzer pin
	buz->pin = p_pin;
	// assign sequence time
	p_sequence_period = (p_sequence_period > MAX_SEQUENCE_TIME) ? MAX_SEQUENCE_TIME : p_sequence_period;
	buz->sequence_period = (p_sequence_period < MIN_SEQUENCE_TIME) ? MIN_SEQUENCE_TIME : p_sequence_period;
	// assign time period of a cycle of buzzing
	p_cycle_period = (p_cycle_period > MAX_PERIOD) ? MAX_PERIOD : p_cycle_period;
	buz->cycle_period = (p_cycle_period < MIN_PERIOD) ? MIN_PERIOD : p_cycle_period;
	// initialize sequence beginning time
	buz->sequence_begin_time = 0;
	// initialize sequence time counter
	buz->time_counter = 0;
}

void execute_sequence(buzzer* buz, unsigned int cycles) {
	unsigned long timenow = millis();
	buz->time_counter = timenow - buz->sequence_begin_time;
	// check if this is the start of a new sequence
	if(buz->time_counter >= buz->sequence_period) {
		buz->sequence_begin_time = timenow;
		buz->time_counter = 0;
	}
	// check progress in current sequence and activate the buzzer accordingly
	// compute the current state the buzzer should be in and change the output
	// of the buzzer's control pin accordingly
	unsigned int i = buz->time_counter / (buz->cycle_period / 2);
	if(i % 2 == 0 || i > (cycles * 2) - 1) {
		buzzerOFF(buz);
	} else {
		buzzerON(buz);
	}
}

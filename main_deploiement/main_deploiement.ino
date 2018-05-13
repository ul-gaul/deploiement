/* 
 * This is the program controlling the parachute deployment system
 * 
 * The program uses a BMP180 to measure altitude and logs its data on an
 * SD card. Various libraries are used to trigger the deployment of the 
 * drogue chute and the main chute. A buzzer is also used to indicate that 
 * the parachute's electronic matches are connected. Finally, all the 
 * configuration values are stored in a seperate configuration file.
 */

// standard libraries includes
#include <SPI.h>
#include <Wire.h>
#include <SD.h>

// project libraries includes
#include "config_deploiement.h"
#include "parachute.h"
#include "buzzer.h"


#define TEST_MODE 0

enum FlightState {
    FLIGHT_LAUNCHPAD,
    FLIGHT_BURNOUT,
    FLIGHT_PRE_DROGUE,
    FLIGHT_PRE_MAIN,
    FLIGHT_DRIFT,
    FLIGHT_LANDED
};

// function declarations
int check_parachutes(parachute* para);


// global variables
FlightState current_flight_state = FLIGHT_LAUNCHPAD;
parachute para_drogue;
parachute para_main;
buzzer state_buzzer;



void setup() {
    init_parachute(&para_drogue, IO_DROGUE_CTRL, IO_DROGUE_STATE);
    init_parachute(&para_main, IO_MAIN_CTRL, IO_MAIN_STATE);
    init_buzzer(&state_buzzer, IO_BUZZER_OUT, BUZZER_TIME_BETWEEN_SEQUENCES,
                BUZZER_CYCLE_DURATION);
}

void loop() {
    switch(current_flight_state) {
        case FLIGHT_LAUNCHPAD:
            check_parachutes(&para_main, &para_drogue, &state_buzzer);
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

int check_parachutes(parachute* p_main, parachute* p_drogue, buzzer* buz) {
    /* get states of the parachutes
     * Truth table is the following:
     *      main    drogue      global state
     *      0       0           0
     *      0       1           1
     *      1       0           2
     *      1       1           3
     */
    unsigned int main_state;
    unsigned int drogue_state;
    main_state = check_connection(p_main);
    drogue_state = check_connection(p_drogue);
    // play appropriate buzzer sequence (parachute state + 1 = number of beeps)
    execute_sequence(buz, main_state*2 + drogue_state + 1);
    // return the global parachute state
    return main_state*2 + drogue_state;
}

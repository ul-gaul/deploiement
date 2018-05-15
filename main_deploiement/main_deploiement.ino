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
#include <TimerOne.h>

// project libraries includes
#include "config_deploiement.h"
#include "parachute.h"
#include "buzzer.h"
#include "sd_logger.h"


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
void request_altitude_update();
unsigned int check_parachutes(parachute* para);
void init_altimeter();
float get_altitude();
float filter_altitude(float raw_alt);
float get_speed();
int update_log_values(sd_log* log);


// global variables
int altitude_up_to_date;
FlightState current_flight_state = FLIGHT_LAUNCHPAD;
parachute para_drogue;
parachute para_main;
unsigned int para_state;
buzzer state_buzzer;
sdlogger_handle sdlogger;
sd_log current_log;
Adafruit_BMP085 altimeter;
float groundPressure;
float raw_altitude_array[ALTITUDE_ARRAY_SIZE];
float filtered_altitude_array[ALTITUDE_ARRAY_SIZE];


void setup() {
    altitude_up_to_date = 0;
    current_log.max_altitude = 0;
    Timer1.initialize(DATA_SAMPLING_PERIOD);
    Timer1.attachInterrupt(request_altitude_update);
    init_parachute(&para_drogue, IO_DROGUE_CTRL, IO_DROGUE_STATE);
    init_parachute(&para_main, IO_MAIN_CTRL, IO_MAIN_STATE);
    init_buzzer(&state_buzzer, IO_BUZZER_OUT, BUZZER_TIME_BETWEEN_SEQUENCES,
                BUZZER_CYCLE_DURATION);
    init_sd_logger(&sdlogger, IO_SD_CS, LOG_UNIT_FILE_NAME);
}

void loop() {
    if(!altitude_up_to_date) {
        // update altitude and other data in the current log
        if(update_log_values(&current_log) == -1) {
            // invalid altitude, log the event
            String event = String("Invalid Altitude");
            log_event(&sdlogger, &current_log, event);
        } else {
            // follow flight plan
            switch(current_flight_state) {
                case FLIGHT_LAUNCHPAD:
                    para_state = check_parachutes(&para_main, &para_drogue, 
                                                  &state_buzzer);
                    // check if burnout started
                    if((current_log.speed > BREAKPOINT_SPEED_TO_BURNOUT) &&
                        (current_log.filtered_altitude > 
                        BREAKPOINT_ALTITUDE_TO_BURNOUT)) {
                        String event = String(MESSAGE_BURNOUT_STARTED);
                        log_event(&sdlogger, &current_log, event);
                        current_flight_state = FLIGHT_BURNOUT;
                    }
                    break;
                case FLIGHT_BURNOUT:
                    // check if burnout done
                    if(current_log.speed < BREAKPOINT_SPEED_TO_PRE_DROGUE) {
                        String event = String(MESSAGE_BURNOUT_FINISHED);
                        log_event(&sdlogger, &current_log, event);
                        current_flight_state = FLIGHT_PRE_DROGUE;
                    }
                    break;
                case FLIGHT_PRE_DROGUE:
                    para_state = check_parachutes(&para_main, &para_drogue, 
                                                  &state_buzzer);
                    // TODO: check if apogee reached, if so: deploy drogue
                    break;
                case FLIGHT_PRE_MAIN:
                    // TODO: check if altitude for main is reached, if so: deploy main
                    para_state = check_parachutes(&para_main, &para_drogue, 
                                                  &state_buzzer);
                    break;
                case FLIGHT_DRIFT:
                    // TODO: check that speed is at landed speed
                    break;
                case FLIGHT_LANDED:
                    check_parachutes(&para_main, &para_drogue, &state_buzzer);
                    break;
            }
        }
        altitude_up_to_date = 1;
    }
}

void request_altitude_update() {
    altitude_up_to_date = 0;
}

unsigned int check_parachutes(parachute* p_main, parachute* p_drogue, 
                              buzzer* buz) {
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

void init_altimeter() {
    altimeter.begin();
    groundPressure = altimeter.readPressure();
}

float get_altitude() {
    return altimeter.readAltitude(groundPressure);
}

float filter_altitude(float raw_alt) {
    // shift the array right
    for (int k = ALTITUDE_ARRAY_SIZE; k > 0; k--){        
        raw_altitude_array[k] = raw_altitude_array[k - 1];
        filtered_altitude_array[k] = filtered_altitude_array[k - 1];
    }
    raw_altitude_array[0] = raw_alt;
    filtered_altitude_array[0] = 0;
    // apply the filter
    for(int i = 0; i < ALTITUDE_ARRAY_SIZE; i++) {
        filtered_altitude_array[0] += (B[i]*raw_altitude_array[i] - 
        A[i]*filtered_altitude_array[i]);
    }
    return filtered_altitude_array[0];
}

float get_speed() {
    float speed = 0;
    for(int i = 0; i < ALTITUDE_ARRAY_SIZE - 1; i++) {
        speed += (filtered_altitude_array[i] - filtered_altitude_array[i + 1]);
    }
    speed = speed/(ALTITUDE_ARRAY_SIZE - 1);
    return (speed >= 0) ? speed : (-1) * speed;
}

int update_log_values(sd_log* log) {
    float tmp_raw_alt = get_altitude();
    // check that altitude is valid
    if(tmp_raw_alt < FLIGHT_MINIMAL_ALTITUDE - 
        ALTIMETER_INVALID_ALTITUDE_TOLERANCE || tmp_raw_alt >  
        FLIGHT_MAXIMAL_ALTITUDE + ALTIMETER_INVALID_ALTITUDE_TOLERANCE) {
        return -1;
    } else {
        log->raw_altitude = tmp_raw_alt;
        log->filtered_altitude = filter_altitude(tmp_raw_alt);
        if(log->filtered_altitude > log->max_altitude) {
            log->max_altitude = log->filtered_altitude;
        }
        log->speed = get_speed();
    }
    return 0;
}

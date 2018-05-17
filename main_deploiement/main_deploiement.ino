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


#define TEST_MODE 1


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
float get_simulation_altitude();


// global variables
int altitude_up_to_date;
unsigned int count_apogee;
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
    count_apogee = 0;
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
                    // check if apogee reached, if so: deploy drogue
                    if(count_apogee >= BREAKPOINT_DELTA_TIME_APOGEE) {
                        if(para_state == TAG_PARACHUTE_NULL || 
                            para_state == TAG_PARACHUTE_MAIN_ONLY) {
                            String event = String(MESSAGE_DROGUE_ALREADY_OUT);
                            log_event(&sdlogger, &current_log, event);
                        }
                        deploy_parachute(&para_drogue);
                        String event = String(MESSAGE_DROGUE_OUT);
                        log_event(&sdlogger, &current_log, event);
                        current_flight_state = FLIGHT_PRE_MAIN;
                    }
                    break;
                case FLIGHT_PRE_MAIN:
                    // check if altitude for main is reached, if so: deploy main
                    para_state = check_parachutes(&para_main, &para_drogue, 
                                                  &state_buzzer);
                    if(current_log.filtered_altitude < BREAKPOINT_ALTITUDE_TO_DRIFT) {
                        if(para_state == TAG_PARACHUTE_NULL) {
                            String event = String(MESSAGE_MAIN_ALREADY_OUT);
                            log_event(&sdlogger, &current_log, event);
                        }
                        deploy_parachute(&para_main);
                        String event = String(MESSAGE_MAIN_OUT);
                        log_event(&sdlogger, &current_log, event);
                        current_flight_state = FLIGHT_DRIFT;
                    }
                    break;
                case FLIGHT_DRIFT:
                    // check that speed is at landed speed
                    if(current_log.speed < BREAKPOINT_SPEED_TO_IDLE) {
                        String event = String(MESSAGE_FLIGHT_FINISHED);
                        log_event(&sdlogger, &current_log, event);
                        current_flight_state = FLIGHT_LANDED;
                    }
                    break;
                case FLIGHT_LANDED:
                    para_state = check_parachutes(&para_main, &para_drogue,
                                                  &state_buzzer);
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
    float tmp_raw_alt;
    if(TEST_MODE) {
        tmp_raw_alt = get_simulation_altitude();
    } else {
        tmp_raw_alt = get_altitude();
    }
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
            count_apogee = 0;
        } else {
            count_apogee++;
        }
        log->speed = get_speed();
    }
    return 0;
}

float get_simulation_altitude() {
    unsigned long t = millis();
    float a;
    switch(current_flight_state) {
        case FLIGHT_LAUNCHPAD:
            a = 3.3652486731274 * pow(10, -5) * t - 36.7480814262348;
            break;
        case FLIGHT_BURNOUT:
            a = -1.76590766247618 * pow(10, -24) * pow(t, 6) + 8.21927550877778 * pow(10, -18) * pow(t, 5) - 1.19590054179482 * pow(10, -11) * pow(t, 4) + 1.91561566553478 * pow(10, -8) * pow(t, 3) + 16.1525398681442 * pow(t, 2) - 15040829.1945499 * t + 4375213557377.89;
            break;
        case FLIGHT_PRE_DROGUE:
            a = -1.5457129719176 * pow(10, -14) * pow(t, -14) * pow(t, 4) + 7.24545784288163 * pow(10, -8) * pow(t, 3) - 0.127364532697129 * pow(t, 2) + 99509.1917079862*t - 29155720076.6889;
            break;
        case FLIGHT_PRE_MAIN:
            a = 2.17265359742678 * pow(10, -8) * pow(t, 2) - 0.0775242158258977*t + 63700.4236312681;
            break;
        case FLIGHT_DRIFT:
            a = -9.91894989006869 * pow(10, -13) * pow(t, 3) + 3.87539476072685 * pow(10, -6) * pow(t, 2) - 5.05446060081143*t + 2200684.64709215;
            break;
        case FLIGHT_LANDED:
            a = -1.5;
            break;
    }
    return a;
}

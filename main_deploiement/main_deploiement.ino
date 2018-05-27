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
    // init buzzer
    init_buzzer(&state_buzzer, IO_BUZZER_OUT, BUZZER_TIME_BETWEEN_SEQUENCES,
                BUZZER_CYCLE_DURATION);
    // init data related variables
    altitude_up_to_date = 0;
    current_log.max_altitude = 0;
    count_apogee = 0;
    // attach interrupt to altitude update function
    Timer1.initialize(DATA_SAMPLING_PERIOD);
    Timer1.attachInterrupt(request_altitude_update);
    // init parachutes
    init_parachute(&para_drogue, IO_DROGUE_CTRL, IO_DROGUE_STATE);
    init_parachute(&para_main, IO_MAIN_CTRL, IO_MAIN_STATE);
    // init sd card
//     pinMode(10, OUTPUT);
//     while(1) {
//         int sderror = init_sd_logger(&sdlogger, IO_SD_CS, LOG_UNIT_FILE_NAME);
//         if(sderror != 0) {
//             for(int i = 0; i < sderror; i++) {
//                 buzzerON(&state_buzzer);
//                 delay(100);
//                 buzzerOFF(&state_buzzer);
//                 delay(100);
//             }
//             delay(500);
//         } else {
//             break;
//         }
//     }
}

void loop() {
//     check_parachutes(&para_main, &para_drogue, &state_buzzer);
    if(!altitude_up_to_date) {
        // update altitude and other data in the current log
        if(update_log_values(&current_log) == -1) {
            // invalid altitude, log the event
            String event = String("Invalid Altitude");
//             log_event(&sdlogger, &current_log, event);
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
//                         log_event(&sdlogger, &current_log, event);
                        current_flight_state = FLIGHT_BURNOUT;
                    }
                    break;
                case FLIGHT_BURNOUT:
                    // check if burnout done
                    if(current_log.speed < BREAKPOINT_SPEED_TO_PRE_DROGUE) {
                        String event = String(MESSAGE_BURNOUT_FINISHED);
//                         log_event(&sdlogger, &current_log, event);
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
//                             log_event(&sdlogger, &current_log, event);
                        }
                        deploy_parachute(&para_drogue);
                        String event = String(MESSAGE_DROGUE_OUT);
//                         log_event(&sdlogger, &current_log, event);
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
//                             log_event(&sdlogger, &current_log, event);
                        }
                        deploy_parachute(&para_main);
                        String event = String(MESSAGE_MAIN_OUT);
//                         log_event(&sdlogger, &current_log, event);
                        current_flight_state = FLIGHT_DRIFT;
                    }
                    break;
                case FLIGHT_DRIFT:
                    // check that speed is at landed speed
                    if(current_log.speed < BREAKPOINT_SPEED_TO_IDLE) {
                        String event = String(MESSAGE_FLIGHT_FINISHED);
//                         log_event(&sdlogger, &current_log, event);
                        current_flight_state = FLIGHT_LANDED;
                    }
                    break;
                case FLIGHT_LANDED:
                    para_state = check_parachutes(&para_main, &para_drogue,
                                                  &state_buzzer);
                    for(int i = 0; i < 10; i++) {
                        buzzerON(&state_buzzer);
                        delay(100);
                        buzzerOFF(&state_buzzer);
                        delay(100);
                    }
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
    execute_sequence(buz, main_state * 2 + drogue_state + 1);
    // return the global parachute state
    return main_state * 2 + drogue_state;
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
            if(t > 16000) {
                a = 3.4932435120246 * pow(10, -22) * pow(t, 6) - 6.0137525133258 * pow(10, -17) * pow(t, 5) + 4.27325583289267 * pow(10, -12) * pow(t, 4) - 1.60300051943013 * pow(10, -7) * pow(t, 3) + 0.00333952376594387 * pow(t, 2) - 36.3492749029025 * t + 160449.656477025;
            } else {
                a = 4.24130141489228 * pow(10, -5) * t + 1.40090814584814;
            }
            break;
        case FLIGHT_BURNOUT:
            if(t > 37200) {
                a = -1.49350189856461 * pow(10, -14) * pow(t, 4) + 2.19414872603138 * pow(10, -9) * pow(t, 3) - 0.000125097528935285 * pow(t, 2) + 3.29683601045748 * t - 31354.7202506453;
            } else {
                a = 3.4932435120246 * pow(10, -22) * pow(t, 6) - 6.0137525133258 * pow(10, -17) * pow(t, 5) + 4.27325583289267 * pow(10, -12) * pow(t, 4) - 1.60300051943013 * pow(10, -7) * pow(t, 3) + 0.00333952376594387 * pow(t, 2) - 36.3492749029025 * t + 160449.656477025;
            }
            break;
        case FLIGHT_PRE_DROGUE:
            if(t > 40500) {
                // simulate next step
                a = 2.172653597427 * pow(10, -8) * pow(t, 2) - 0.0282022850738493 * t + 3697.35684913407;
            } else {
                a = -1.49350189856461 * pow(10, -14) * pow(t, 4) + 2.19414872603138 * pow(10, -9) * pow(t, 3) - 0.000125097528935285 * pow(t, 2) + 3.29683601045748*t - 31354.7202506453;
            }
            break;
        case FLIGHT_PRE_MAIN:
            if(t > 125100) {
                // simulate next step
                a = a = -9.91894988952945 * pow(10, -13) * pow(t, 3) + 4.97807830666201 * pow(10, -6) * pow(t, 2) - 0.0906045209884312 * t + 5961.90970204993;
            } else {
                a = 2.172653597427 * pow(10, -8) * pow(t, 2) - 0.0282022850738493 * t + 3697.35684913407;
            }
            break;
        case FLIGHT_DRIFT:
            if(t > 180000) {
                // simulate next step
                a = 0;
            } else {
                a = -9.91894988952945 * pow(10, -13) * pow(t, 3) + 4.97807830666201 * pow(10, -6) * pow(t, 2) - 0.0906045209884312 * t + 5961.90970204993;
                a = (a >= 0) ? a : 0;
            }
            break;
        case FLIGHT_LANDED:
            a = 0;
            break;
    }
    return a;
}

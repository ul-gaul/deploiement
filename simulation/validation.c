/* 
 * This program emulates the deployment algorithm
 */

// standard libraries includes
#include <stdio.h>
#include <math.h>

// project libraries includes
#include "config_deploiement.h"


enum FlightState {
    FLIGHT_LAUNCHPAD,
    FLIGHT_BURNOUT,
    FLIGHT_PRE_DROGUE,
    FLIGHT_PRE_MAIN,
    FLIGHT_DRIFT,
    FLIGHT_LANDED
};

typedef struct {
    int drogue;
    int main;
} Parachutes;

typedef struct {
    char logbuffer[512];
    float raw_altitude;
    float filtered_altitude;
    float max_altitude;
    float speed;
} csv_log;

// function declarations
void loop(unsigned long t, FILE* f);
void request_altitude_update();
unsigned int check_parachutes(Parachutes* para);
void init_altimeter();
float get_altitude();
float filter_altitude(float raw_alt);
float get_speed();
int update_log_values(unsigned long t, csv_log* log);
float get_simulation_altitude(unsigned long t);


// global variables
int altitude_up_to_date;
unsigned int count_apogee;
enum FlightState current_flight_state = FLIGHT_LAUNCHPAD;
csv_log current_log;
Parachutes parachutes;
unsigned int para_state;
float groundPressure;
float raw_altitude_array[ALTITUDE_ARRAY_SIZE];
float filtered_altitude_array[ALTITUDE_ARRAY_SIZE];


int main() {
    // init data related variables
    altitude_up_to_date = 0;
    current_log.max_altitude = 0;
    count_apogee = 0;
    parachutes.drogue = 0;
    parachutes.main = 0;
    
    FILE* log_file = fopen("log.csv", "w");
    if(log_file == NULL) {
        perror("can't open file");
        return -1;
    }
    fprintf(log_file,"t,current_flight_state,drogue,main,raw_altitude,filtered_altitude,speed,message\n");
    
    for(unsigned long t = 0; t < 181000; t++) {
        loop(t, log_file);
    }
    fclose(log_file);
    return 0;
}

void loop(unsigned long t, FILE* f) {
    // update altitude and other data in the current log
    if(update_log_values(t, &current_log) == -1) {
        // invalid altitude, log the event
//             String event = String("Invalid Altitude");
        //             log_event(&sdlogger, &current_log, event);
        fprintf(f, "%lu,%d,%d,%d,%f,%f,%f,invalid altitude\n", t, 
                current_flight_state, parachutes.drogue,
                parachutes.main, current_log.raw_altitude, 
                current_log.filtered_altitude, current_log.speed);
    } else {
        if(t % 100 == 0) {
            fprintf(f, "%lu,%d,%d,%d,%f,%f,%f,\n", t, current_flight_state, 
                parachutes.drogue, parachutes.main, current_log.raw_altitude, 
                current_log.filtered_altitude, current_log.speed);
        }
        // follow flight plan
        switch(current_flight_state) {
            case FLIGHT_LAUNCHPAD:
                para_state = check_parachutes(&parachutes);
                // check if burnout started
                if((current_log.speed > BREAKPOINT_SPEED_TO_BURNOUT) &&
                    (current_log.filtered_altitude > 
                    BREAKPOINT_ALTITUDE_TO_BURNOUT)) {
                    fprintf(f, "%lu,%d,%d,%d,%f,%f,%f,burnout started\n", t, 
                            current_flight_state, parachutes.drogue,
                            parachutes.main, current_log.raw_altitude, 
                            current_log.filtered_altitude, current_log.speed);
                    current_flight_state = FLIGHT_BURNOUT;
                }
                break;
            case FLIGHT_BURNOUT:
                // check if burnout done
                if(current_log.speed < BREAKPOINT_SPEED_TO_PRE_DROGUE) {
                    fprintf(f, "%lu,%d,%d,%d,%f,%f,%f,burnout finished\n", t, 
                            current_flight_state, parachutes.drogue,
                            parachutes.main, current_log.raw_altitude, 
                            current_log.filtered_altitude, current_log.speed);
                    current_flight_state = FLIGHT_PRE_DROGUE;
                }
                break;
            case FLIGHT_PRE_DROGUE:
                para_state = check_parachutes(&parachutes);
                // check if apogee reached, if so: deploy drogue
                if(count_apogee >= BREAKPOINT_DELTA_TIME_APOGEE) {
                    if(para_state == TAG_PARACHUTE_NULL || 
                        para_state == TAG_PARACHUTE_MAIN_ONLY) {
                        fprintf(f, "%lu,%d,%d,%d,%f,%f,%f,drogue already out\n", t, 
                                current_flight_state, parachutes.drogue,
                                parachutes.main, current_log.raw_altitude, 
                                current_log.filtered_altitude, current_log.speed);
                        }
                        parachutes.drogue = 1;
                        fprintf(f, "%lu,%d,%d,%d,%f,%f,%f,drogue out\n", t, 
                                current_flight_state, parachutes.drogue,
                                parachutes.main, current_log.raw_altitude, 
                                current_log.filtered_altitude, current_log.speed);
                        current_flight_state = FLIGHT_PRE_MAIN;
                }
                break;
            case FLIGHT_PRE_MAIN:
                // check if altitude for main is reached, if so: deploy main
                para_state = check_parachutes(&parachutes);
                if(current_log.filtered_altitude < BREAKPOINT_ALTITUDE_TO_DRIFT) {
                    if(para_state == TAG_PARACHUTE_NULL) {
                        fprintf(f, "%lu,%d,%d,%d,%f,%f,%f,main already out\n", t, 
                                current_flight_state, parachutes.drogue,
                                parachutes.main, current_log.raw_altitude, 
                                current_log.filtered_altitude, current_log.speed);
                    }
                    parachutes.main = 1;
                    fprintf(f, "%lu,%d,%d,%d,%f,%f,%f,main out\n", t, 
                            current_flight_state, parachutes.drogue,
                            parachutes.main, current_log.raw_altitude, 
                            current_log.filtered_altitude, current_log.speed);
                    current_flight_state = FLIGHT_DRIFT;
                }
                break;
            case FLIGHT_DRIFT:
                // check that speed is at landed speed
                if(current_log.speed < BREAKPOINT_SPEED_TO_IDLE) {
                    fprintf(f, "%lu,%d,%d,%d,%f,%f,%f,flight finished\n", t, 
                            current_flight_state, parachutes.drogue,
                            parachutes.main, current_log.raw_altitude, 
                            current_log.filtered_altitude, current_log.speed);
                    current_flight_state = FLIGHT_LANDED;
                }
                break;
            case FLIGHT_LANDED:
                para_state = check_parachutes(&parachutes);
                break;
        }
    }
}

void request_altitude_update() {
    altitude_up_to_date = 0;
}

unsigned int check_parachutes(Parachutes* para) {
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
    main_state = para->drogue;
    drogue_state = para->main;
    // return the global parachute state
    return main_state * 2 + drogue_state;
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

int update_log_values(unsigned long t, csv_log* log) {
    float tmp_raw_alt;
    tmp_raw_alt = get_simulation_altitude(t);
    // check that altitude is valid
    if(tmp_raw_alt < FLIGHT_MINIMAL_ALTITUDE - 
        ALTIMETER_INVALID_ALTITUDE_TOLERANCE || tmp_raw_alt >  
        FLIGHT_MAXIMAL_ALTITUDE + ALTIMETER_INVALID_ALTITUDE_TOLERANCE) {
        printf("invalid altitude: %f\n", tmp_raw_alt);
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


float get_simulation_altitude(unsigned long t) {
    float a;
    /*
     * some simulations step changes are time triggered:
     *      burnout, predrogue and landed
     * while the others are triggered by the current step of the flight
     */
    switch(current_flight_state) {
        case FLIGHT_LAUNCHPAD:
            if(t >= 17000) {
                t = t - 17000;
                a = -1.50157070738923 * pow(10, -10) * pow(t, 3) - 5.07498255323106 * pow(10, -7) * pow(t, 2) + 0.19493487583132 * t - 11.8122615878982;
            } else {
                a = 4.24130141489228 * pow(10, -5) * t + 1.40090814584814;
            }
            break;
        case FLIGHT_BURNOUT:
            if(t >= 37200) {
                t = t - 37116;
                a = -1.29739044869357 * pow(10, -10) * pow(t, 3) + 2.63704851215904 * pow(10, -6) * pow(t, 2) + 0.0466018454543128 * t + 1605.2401465559;
            } else {
                t = t - 17000;
                a = -1.50157070738923 * pow(10, -10) * pow(t, 3) - 5.07498255323106 * pow(10, -7) * pow(t, 2) + 0.19493487583132 * t - 11.8122615878982;
            }
            break;
        case FLIGHT_PRE_DROGUE:
            t = t - 37116;
            a = -1.29739044869357 * pow(10, -10) * pow(t, 3) + 2.63704851215904 * pow(10, -6) * pow(t, 2) + 0.0466018454543128 * t + 1605.2401465559;
            break;
        case FLIGHT_PRE_MAIN:
            t = t - 40700;
            a = -0.0246021995362092 * t + 3473.00742330786;
            break;
        case FLIGHT_DRIFT:
            if(t > 180000) {
                // simulate next step
                a = 0;
            } else {
                t = t - 125000;
                a = -0.0084133394471041 * t + 788.869942821509;
            }
            break;
        case FLIGHT_LANDED:
            a = 0;
            break;
    }
    a = (a >= 0) ? a : 1.0;
    return a;
}



// old simulation
// float get_simulation_altitude(unsigned long t) {
//     float a;
//     /*
//     * some simulations step changes are time triggered:
//     *      burnout, predrogue and landed
//     * while the others are triggered by the current step of the flight
//     */
//     switch(current_flight_state) {
//         case FLIGHT_LAUNCHPAD:
//             if(t > 16000) {
//                 a = 3.4932435120246 * pow(10, -22) * pow(t, 6) - 6.0137525133258 * pow(10, -17) * pow(t, 5) + 4.27325583289267 * pow(10, -12) * pow(t, 4) - 1.60300051943013 * pow(10, -7) * pow(t, 3) + 0.00333952376594387 * pow(t, 2) - 36.3492749029025 * t + 160449.656477025;
//             } else {
//                 a = 4.24130141489228 * pow(10, -5) * t + 1.40090814584814;
//             }
//             break;
//         case FLIGHT_BURNOUT:
//             if(t > 37200) {
//                 a = -1.49350189856461 * pow(10, -14) * pow(t, 4) + 2.19414872603138 * pow(10, -9) * pow(t, 3) - 0.000125097528935285 * pow(t, 2) + 3.29683601045748 * t - 31354.7202506453;
//             } else {
//                 a = 3.4932435120246 * pow(10, -22) * pow(t, 6) - 6.0137525133258 * pow(10, -17) * pow(t, 5) + 4.27325583289267 * pow(10, -12) * pow(t, 4) - 1.60300051943013 * pow(10, -7) * pow(t, 3) + 0.00333952376594387 * pow(t, 2) - 36.3492749029025 * t + 160449.656477025;
//             }
//             break;
//         case FLIGHT_PRE_DROGUE:
//             //             if(t > 40500) {
//             //                 // simulate next step
//             //                 a = 2.172653597427 * pow(10, -8) * pow(t, 2) - 0.0282022850738493 * t + 3697.35684913407;
//             //             } else {
//             a = -1.49350189856461 * pow(10, -14) * pow(t, 4) + 2.19414872603138 * pow(10, -9) * pow(t, 3) - 0.000125097528935285 * pow(t, 2) + 3.29683601045748*t - 31354.7202506453;
//             //             }
//             break;
//         case FLIGHT_PRE_MAIN:
//             //             if(t > 125100) {
//             //                 // simulate next step
//             //                 a = -9.91894988952945 * pow(10, -13) * pow(t, 3) + 4.97807830666201 * pow(10, -6) * pow(t, 2) - 0.0906045209884312 * t + 5961.90970204993;
//             //             } else {
//             a = 2.172653597427 * pow(10, -8) * pow(t, 2) - 0.0282022850738493 * t + 3697.35684913407;
//             //             }
//             break;
//         case FLIGHT_DRIFT:
//             if(t > 180000) {
//                 // simulate next step
//                 a = 0;
//             } else {
//                 a = -9.91894988952945 * pow(10, -13) * pow(t, 3) + 4.97807830666201 * pow(10, -6) * pow(t, 2) - 0.0906045209884312 * t + 5961.90970204993;
//                 a = (a >= 0) ? a : 0;
//             }
//             break;
//         case FLIGHT_LANDED:
//             a = 0;
//             break;
//     }
//     return a;
// }
// 

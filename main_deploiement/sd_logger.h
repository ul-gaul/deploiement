/*
 * Library used to interface the SD card and handle logs during the flight
 */

// standard libraries includes
#include <SD.h>

// project related includes
#include "config_deploiement.h"

// struct used to handle the SD logger
typedef struct {
    byte cspin;
    File file_handle;
    char logfilename[512];
} sdlogger_handle;


typedef struct {
    char logbuffer[512];
    float raw_altitude;
    float filtered_altitude;
    float max_altitude;
    float speed;
} sd_log;

void init_sd_logger(sdlogger_handle* sdlogger, byte chip_select_pin, 
                    char* logfilename);

void log_data(sdlogger_handle* logger, sd_log* log);

void log_event(sdlogger_handle* logger, sd_log* log, String message);

void close_sd_logger(sdlogger_handle* logger);

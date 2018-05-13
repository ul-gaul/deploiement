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
    char logfilename[256];
} sdlogger_handle;


void init_sd_logger(sdlogger_handle* sdlogger, byte chip_select_pin, 
                    char* logfilename);

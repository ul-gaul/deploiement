#include "sd_logger.h"

void init_sd_logger(sdlogger_handle* sdlogger, byte chip_select_pin, 
                    char* logfilename) {
    SD.begin(chip_select_pin);
    sdlogger->cspin = chip_select_pin;
    // generate filename, credit goes to Jonathan Neault for the algorithm
    for(int i = 1; i <= LOG_UNIT_MAX_NB_OF_FILES; i++) {
        sprintf(sdlogger->logfilename, "%s_%d%s", logfilename, i, 
                LOG_UNIT_FILE_EXT);
        if(!SD.exists(sdlogger->logfilename)) {
            break;
        }
    }
    sdlogger->file_handle = SD.open(sdlogger->logfilename, FILE_WRITE);
    // write header of file, need to use the String type since Arduino's 
    // println function does not support char arrays
    String header = String(ID_LOG_MESSAGE) + 
            String(",timestamp,rawAltitude,filteredAltitude,speed,message");
    sdlogger->file_handle.println(header);
}

void log_data(sdlogger_handle* logger, sd_log* log) {
    
    logger->file_handle.println(String(log->logbuffer));
    logger->file_handle.flush();
}

void log_event(sdlogger_handle* logger, sd_log* log, String message) {
    
    logger->file_handle.println(String(log->logbuffer));
    logger->file_handle.flush();
}

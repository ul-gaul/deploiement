#include "buzzer.h"

Buzzer::Buzzer() {
  
}

void Buzzer::init(byte arduinoPin, int sequenceDuration, int soundDuration) { 
    if(sequenceDuration < 5000) {
        sequenceDuration = 5000;
    }
    
    else if(sequenceDuration > 10000) {
        sequenceDuration = 10000;
    }
    
    if(soundDuration < 500) {
        soundDuration = 500;
    }
    
    else if(soundDuration > 1500) {
        soundDuration = 1500;
    }
    
    _buzzerControlPin = arduinoPin;
    _sequenceDuration = sequenceDuration;
    _cycleDuration = soundDuration;
    
    pinMode(_buzzerControlPin, OUTPUT);
    turnOff();
}

void Buzzer::executeBuzzerSequence(byte numberOfCycle) {
/*    
 *     La fonction executeBuzzerSequence n'est pas blocante, c'est-à-dire que le programme
 *     principal peut continuer à travailler pendant que la librairie gère la séquence en
 *     cours. La fonction executeBuzzerSequence gère la séquence en vérifiant les indices
 *     de temps notée par rapport aux paramètres de la séquence qui caractérise l'objet buzzer.
 */
    int timeNow = millis();
    int sequenceDurationCounter = timeNow - _sequenceBeginningTimestamp;

    // Vérification de si on doit recommencer une nouvelle séquence
    if(sequenceDurationCounter > _sequenceDuration) {
        _sequenceBeginningTimestamp = timeNow;
        sequenceDurationCounter = 0;
    }

    // Vérification de l'étape où le buzzer est dans la séquence
    if(sequenceDurationCounter > 7*_cycleDuration/2 && numberOfCycle > 3) {
        turnOff();
    }
    else if(sequenceDurationCounter > 6*_cycleDuration/2 && numberOfCycle > 3) {
        turnOn();
    }    
    
    else if(sequenceDurationCounter > 5*_cycleDuration/2 && numberOfCycle > 2) {
        turnOff();
    }
    else if(sequenceDurationCounter > 4*_cycleDuration/2 && numberOfCycle > 2) {
        turnOn();
    }
    
    else if(sequenceDurationCounter > 3*_cycleDuration/2 && numberOfCycle > 1) {
        turnOff();
    }
    else if(sequenceDurationCounter > 2*_cycleDuration/2 && numberOfCycle > 1) {
        turnOn();
    }
    
    else if(sequenceDurationCounter > _cycleDuration/2) {
        turnOff();
    }
    else if(sequenceDurationCounter >= 0) {
        turnOn();
    }
}

void Buzzer::turnOn() {
    digitalWrite(_buzzerControlPin, 1);
}

void Buzzer::turnOff() {
    digitalWrite(_buzzerControlPin, 0);
}

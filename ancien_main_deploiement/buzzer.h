/*  
 * Créé par Dominic Caron le 2016-02-29.  
 * Ce module permet d'interfacer un buzzer simple sur un Arduino. Le buzzer est controlé de façon binaire,
 * il est activé tant qu'on lui fournit un 3.3V (High) et il est éteint avec un 0V (Low). 
 * 
 * Les paramètres d'initialisation sont:
 *    - Le numéro de pin du Arduino
 *    - La durée d'une séquence.
 *        Entre 5000 et 10000 ms. Ces valeurs ont été déterminé de façon arbitraire
 *    - La durée d'un cycle (High suivit d'un Low sur le signal de control)
 *        Entre 500 et 1500 ms. Ces valeurs ont été déterminé de façon arbitraire 
 *        
 * Une séquence est composée de un ou plusieurs cycles suivit d'un moment de silence tampon. Un cycle 
 * correspond à une période d'un signal carré. 
 * 
 * Exemple d'une séquence de durée déterminé composé de 1 cycle, 2 cycles et 4 cycles: 
 *    (High, Low),  Low,  Low,   Low,  Low,  Low, Low, Low, Low, Low, Low
 *    (High, Low), (High, Low),  Low,  Low,  Low, Low, Low, Low, Low, Low
 *    (High, Low), (High, Low), (High, Low), Low, Low, Low, Low, Low, Low
 */
 
#ifndef buzzer_h
#define buzzer_h

#include "Arduino.h"

class Buzzer {   
    public:
        Buzzer();
        void init(byte arduinoPin, int sequenceDuration, int soundDuration);
        void turnOn();
        void turnOff();
        void executeBuzzerSequence(byte numberOfCycle);
    
    private:    
        byte _buzzerControlPin;
        int _sequenceDuration;
        int _cycleDuration;
        int _sequenceBeginningTimestamp;        
};
#endif /* buzzer_h */

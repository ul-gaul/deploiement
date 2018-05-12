/*  
 * Créé par Dominic Caron le 2016-02-28.  
 * Ce module est une interface de control pour le système de déploiement des parachutes d'une fusée.
 * Celui-ci intègre tout les modules de hardware et offre des fonctions qui permettent à la fusée de
 * faire les tâches nécessaires à un vol complet en offrant un niveau d'abstraction pour l'utilisateur.
 */

#ifndef _rocket_h
#define _rocket_h

#include "configCircuitDeploiement.h"

class Rocket {
    public:
        Rocket();
        float getSpeed();
        float getAltitude(byte index);
        float getMaxAltitude();
        
        void initHardware();        
        bool updateAltitude();     
        void logData();
        void logEvent(String message);    
        void deployParachute(bool parachuteId);
        byte verifyParachutes();
        void stopLogging();

   
    private:
        float _mesuredAltitude[ALTITUDE_ARRAY_SIZE];
        float _filteredAltitude[ALTITUDE_ARRAY_SIZE];
        float _maxAltitude;
        float _speed;
        
        float _groundPressure;
        
        Adafruit_BMP085 _altimeter;
        File _logFile;
        Buzzer _buzzer;
        Match _drogueParachute;
        Match _mainParachute;
        
        void _initLogUnit(byte chipSelectPin, long serialBaudRate, String fileName);
        void _initAltimeter();

        bool _validateAltitude(float mesuredAltitude);
        void _prepareAltitudeVector();
        void _filterAltitude();
        void _calculateSpeed();
        void _verifyMaxAltitude();
};
#endif

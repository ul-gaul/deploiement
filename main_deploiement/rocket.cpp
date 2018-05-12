#include "rocket.h"

Rocket::Rocket() {
/*
 * Le constructeur initialise les variables qui contiendront les valeurs d'altitudes mesurées
 * et filtrés. Il est important de bien les initialiser à la valeur 0.
 */
    for(int i = 0; i < ALTITUDE_ARRAY_SIZE; i++) {
        _mesuredAltitude[i] = 0;
        _filteredAltitude[i] = 0;
    }
    _maxAltitude = 0;
    _speed = 0;
    _groundPressure = 0;
}


float Rocket::getSpeed() {
    return _speed;
}

float Rocket::getAltitude(byte index) {
    return _filteredAltitude[index];
}

float Rocket::getMaxAltitude() {
    return _maxAltitude;
}

void Rocket::initHardware() {
    _initLogUnit(IO_SD_CS, LOG_UNIT_SERIAL_BAUDRATE, LOG_UNIT_FILE_NAME);
    _initAltimeter();    
    _buzzer.init(IO_BUZZER_OUT, BUZZER_TIME_BETWEEN_SEQUENCES, BUZZER_CYCLE_DURATION);  
    _drogueParachute.init(IO_DROGUE_OUT, IO_DROGUE_FEEDBACK);
    _mainParachute.init(IO_MAIN_OUT, IO_MAIN_FEEDBACK);
}

bool Rocket::updateAltitude() {
/*
 * La mise à jour de l'altitude de la fusée se fait en plusieurs étapes à cause du filtre
 * passe bas que l'on applique sur les valeurs d'altitudes mesurées. Pour plus de détails
 * sur le filtre, voir le fichier de configuration ou le rapport de réalisation approprié.
 */
    bool validAltitude;
    float mesuredAltitude;

    mesuredAltitude = _altimeter.readAltitude(_groundPressure);
    validAltitude = _validateAltitude(mesuredAltitude);
    
    if(validAltitude) {
        _prepareAltitudeVector();
        _mesuredAltitude[0] = mesuredAltitude;
        _filterAltitude();
        _calculateSpeed();
        _verifyMaxAltitude();
    }
    return validAltitude;
}

void Rocket::logData() {
    String dataStream;
    dataStream += String(ID_LOG_DATA);
    dataStream += (",");
    dataStream += String(millis());
    dataStream += (",");
    dataStream += String(_mesuredAltitude[0]);
    dataStream += (",");
    dataStream += String(_filteredAltitude[0]);
    dataStream += (",");
    dataStream += String(_speed);
    
    Serial.println(dataStream);
    if (_logFile) {
        _logFile.println(dataStream);
        _logFile.flush(); // Écrit le data physiquement sur la carte
    }
}

void Rocket::logEvent(String message) {
    String dataStream;
    dataStream += String(ID_LOG_EVENT);
    dataStream += (",");
    dataStream += String(millis());
    dataStream += (",");
    dataStream += String(_mesuredAltitude[0]);
    dataStream += (",");
    dataStream += String(_filteredAltitude[0]);
    dataStream += (",");
    dataStream += String(_speed);
    dataStream += (",");
    dataStream += message;
    
    Serial.println(dataStream);
    if (_logFile) {
        _logFile.println(dataStream);
    }
}

void Rocket::deployParachute(bool parachuteId) {
    switch(parachuteId) {
        case 0:
            _drogueParachute.lightMatch();
            break;
            
        case 1:
            _mainParachute.lightMatch();
            break;
    }
}

byte Rocket::verifyParachutes() {
/*
 * Vérifie la connection des parachutes et envoi un signal sonore selon l'état des parachutes.
 */
    byte parachutesState = 0;
    if(_drogueParachute.verifyMatchConnection() && _mainParachute.verifyMatchConnection()) {
        parachutesState = TAG_PARACHUTE_BOTH;
        _buzzer.executeBuzzerSequence(4);
    }
    
    else if(_mainParachute.verifyMatchConnection()) {
        parachutesState = TAG_PARACHUTE_MAIN_ONLY;
        _buzzer.executeBuzzerSequence(3);
    }
           
    else if(_drogueParachute.verifyMatchConnection()) {
        parachutesState = TAG_PARACHUTE_DROGUE_ONLY;
        _buzzer.executeBuzzerSequence(2);
    }
    
    else {
        parachutesState = TAG_PARACHUTE_NULL;
        _buzzer.executeBuzzerSequence(1);
    }
    return parachutesState;
}

void Rocket::stopLogging() {
    _logFile.close();
    Serial.end();
}


//------------------------------------------------------------------------------------------------------------------------
// Méthodes privées

void Rocket::_initAltimeter() {
/*
 * Cette fonction initialise l'altimètre en appelant la méthode contenu dans son driver. L'initialisation
 * comprend aussi le réglage de la pression de référence de l'altimère à l'aide de la variable _groundPressure. 
 */
    _altimeter.begin();
    _groundPressure = _altimeter.readPressure();
}

void Rocket::_initLogUnit(byte chipSelectPin, long serialBaudRate, String fileName) {
/*
 * Cette fonction initialise les périphériques d'enregistrement de l'historique du vol
 * de la fusée. Les périphériques sont le port série pour le débug et la carte Sd.
 * 
 * Attention: Laisser le port série activée n'est pas un bonne pratique! On peut faire
 *            une exception dans ce cas-ci, car les programmes utilisés ne sont pas 
 *            très gourmant en ressource. De plus, le risque de se faire hacker est
 *            presque nul.
 */
    Serial.begin(serialBaudRate);
    
    // Initialisation de la carte SD
    SD.begin(chipSelectPin);
    for (int i = 1; i <= LOG_UNIT_MAX_NB_OF_FILES; i++) {
        // Génération d'un nom de fichier unique sur la carte SD (Merci Jonathan Neault)
        String fileNameTemp = fileName + '_' + String(i) + LOG_UNIT_FILE_EXT;
        if (!SD.exists(fileNameTemp)) {
            fileName = fileNameTemp;
            break;
        }
    }
    _logFile = SD.open(fileName, FILE_WRITE);
  
    String dataStream;
    dataStream += String(ID_LOG_MESSAGE);
    dataStream += String(",");
    dataStream += String("timeStamp,rawAltitude,filteredAltitude,speed,message");
    Serial.println(dataStream);    
    if (_logFile) {
        _logFile.println(dataStream);
    }
}

void Rocket::_prepareAltitudeVector() {
/*
 * Décale les vecteurs contenant les valeurs d'altitudes passées de 1 vers la droite pour
 * les préparer à la prochaine entré d'altitude. Il est important de définir la valeur
 * d'altitude présente (_mesuredAltitude[0]) à 0 avant la mesure d'altitude pour éviter des
 * problèmes lors du filtrage.
 *              Avant: [y0, y1, y2, y3]    Après: [0, y0, y1, y2]
 */
    for(int i = ALTITUDE_ARRAY_SIZE - 1; i > 0; i--) {    
        _mesuredAltitude[i] = _mesuredAltitude[i-1];
        _filteredAltitude[i] = _filteredAltitude[i-1];
    }
    _mesuredAltitude[0] = 0;
    _filteredAltitude[0] = 0;
}

bool Rocket::_validateAltitude(float mesuredAltitude) {
/*
 *  Prend une valeur d'altitude et vérifie si elle correspond à la valeur retourné par les drivers
 *  du Bmp180 lors d'une réponse invalide de celui-ci.
 */
    bool validAltitude;
    
    if(mesuredAltitude < FLIGHT_MINIMAL_ALTITUDE - ALTIMETER_INVALID_ALTITUDE_TOLERANCE |
       mesuredAltitude > FLIGHT_MAXIMAL_ALTITUDE + ALTIMETER_INVALID_ALTITUDE_TOLERANCE) {
        validAltitude = false;
    }
    else {
        validAltitude = true;
    }
    return validAltitude;
}

void Rocket::_filterAltitude() {
/*
 * Filtre la valeur d'altitude mesurée en calculant l'équation aux différence. Pour plus de détails
 * voir fichier configCircuitDeploiement.h ou le rapport de réalisation de la fusée.
 */
    for(int i = 0; i < ALTITUDE_ARRAY_SIZE; i++) {
        _filteredAltitude[0] += (B[i]*_mesuredAltitude[i] - A[i]*_filteredAltitude[i]);
    }
}

void Rocket::_calculateSpeed() {
/*
 * Calcul la vitesse instantannée de la fusée avec la dérivé de l'altitude (différence d'altitude selon le temps)
 * On calcul plusieurs vitesses différentes à l'aide des valeurs d'altitude contenu dans le vecteur et on fait la
 * moyenne des vitesses.
 */
    _speed = 0;
    for(int i = 0; i < ALTITUDE_ARRAY_SIZE-1; i++) {
        _speed += (_filteredAltitude[i] - _filteredAltitude[i+1]);
    }
    _speed = _speed/(ALTITUDE_ARRAY_SIZE-1);
        
    if(_speed < 0) {
        _speed = -_speed;
    }
}

void Rocket::_verifyMaxAltitude() {
/*
 * Vérifie si l'altitude instantannée est la plus grand atteinte pendant ce vol.
 */
    if(_filteredAltitude[0] > _maxAltitude) {
        _maxAltitude = _filteredAltitude[0];
    }
}

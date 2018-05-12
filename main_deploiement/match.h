/*  
 * Créé par Maxime Guillemette le 10-03-2016 
 * Ce module permet d'interfacer une allumette électronique à l'aide d'un Arduino. Le Arduino
 * controle un transistor qui agit comme intérupteur pour court-circuiter l'allumette. Il est 
 * aussi possible de lire l'état de l'allumette pour savoir si elle a explosé ou non. La
 * fonctionnalité de lecture fonctionne doit aussi être implémenter sur le circuit électrique
 * pour bien fonctionner. Voir le schéma du circuit de déploiement.
 */

#ifndef MATCH_H
#define MATCH_H

#include "Arduino.h"

class Match {
    public:
        Match();
        void init(byte controlPin, byte statePin);
        void lightMatch();
        bool verifyMatchConnection();

    private:
        byte _controlPin;
        byte _feedbackPin;
};
#endif // MATCH_H

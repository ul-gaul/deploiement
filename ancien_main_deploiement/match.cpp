#include "match.h"

Match::Match() {
    
}

void Match::init(byte controlPin, byte feedbackPin) {
    _controlPin = controlPin;
    _feedbackPin = feedbackPin;
    
    pinMode(_feedbackPin, INPUT);
    pinMode(_controlPin, OUTPUT);
    digitalWrite(_controlPin, LOW);
}

void Match::lightMatch() {
    digitalWrite(_controlPin, HIGH);
}

bool Match::verifyMatchConnection() {
    return digitalRead(_feedbackPin);
}

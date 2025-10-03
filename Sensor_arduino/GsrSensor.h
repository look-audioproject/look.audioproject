#ifndef SENSOR_H
#define SENSOR_H

#include <Arduino.h>
#include "LookConstants.h"

class GsrSensor
{
public:
    GsrSensor(int pin);
    int baseValue();
    int averageValue();
    int averageValueForLastSecond();
    void loop();
    void reset();
    int averageValueForLastSeconds(int nrOfSeconds, bool debug=false);
private:
    int _determineAverage();
    int _determineAverageForSecond();
    void _resetPerSecondValues();
    void _processSensor();
    void _processSensorForSeconds();
    static constexpr byte READ_SENSOR_INTERVAL = 5;
    static constexpr byte SENSOR_MAX_PER_SECOND_COUNT = 10;
    static constexpr byte SENSOR_MAX_SECONDS_COUNT = 20;
    static constexpr byte SENSOR_MAX_COUNT = 10;
    int _baseValue=0;
    int _averageValue=0;
    int _pin;
    int _sensorIndex = 0;
    int _sensorSecondsIndex = 0;
    int _sensorLastSecondsIndex = 0;
    int _sensorValues[10] = {0,0,0,0,0,0,0,0,0,0};
    int _sensorSecondsValues[10] = {0,0,0,0,0,0,0,0,0,0};
    int _sensorLastSecondsValues[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    unsigned long _currReadTime=0;
    unsigned long _currReadSecondsTime=0;    
};

#endif
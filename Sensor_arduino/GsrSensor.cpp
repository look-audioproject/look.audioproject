#include <Arduino.h>
#include "LookConstants.h"
#include "GsrSensor.h"

GsrSensor::GsrSensor(int pin)
{
    _currReadTime = millis();
    _currReadSecondsTime = millis();
    _pin = pin;
}

int GsrSensor::averageValue()
{
    return _averageValue;
}

int GsrSensor::averageValueForLastSecond()
{
	return _determineAverageForSecond();
}

int GsrSensor::averageValueForLastSeconds(int nrOfSeconds, bool debug=false)
{
	if (nrOfSeconds <= SENSOR_MAX_SECONDS_COUNT && nrOfSeconds > 0)
	{
		int sum=0;
		int nrOfValues=0;
			
		int nextIndex = _sensorLastSecondsIndex - 2; // the _sensorLastSecondsIndex is actually for the next second position,
													// and we need to count the average exclusive the current one
		if (nextIndex < 0)
		{
			nextIndex = SENSOR_MAX_SECONDS_COUNT + nextIndex;
		}
		
		for (int i=nrOfSeconds;i>0;i--) {
			if (_sensorLastSecondsValues[nextIndex] > 0) {
				sum += _sensorLastSecondsValues[nextIndex];
				nrOfValues += 1;

				if (debug)
				{
					Serial.print(_sensorLastSecondsValues[nextIndex]);
					Serial.print(", ");
				}

				nextIndex=nextIndex-1;
				
				if ( nextIndex < 0 ) {
					nextIndex = SENSOR_MAX_SECONDS_COUNT + nextIndex;
				}
				
			} else {
				break; // apparently we didn't have any prior measure, so leave it.
			}			
		}
		return nrOfValues > 0 ? ( sum / nrOfValues ) : averageValue();
	}
	return averageValue(); // else just return the current
}

int GsrSensor::baseValue()
{
    return _baseValue;
}

int GsrSensor::_determineAverage()
{
    int sum=0;
    for(int i=0;i<SENSOR_MAX_COUNT;i++)           //Average the 10 measurements to remove the glitch
        {
        sum += _sensorValues[i];
        }
    return sum/SENSOR_MAX_COUNT;
}

int GsrSensor::_determineAverageForSecond()
{
    int sum=0;
	int nrOfValues=0;
    for(int i=0;i<SENSOR_MAX_PER_SECOND_COUNT;i++) //Average the last non-zero measurements
    {
		  if (_sensorSecondsValues[i] > 0) {		
	      sum += _sensorSecondsValues[i];
			  nrOfValues += 1;
		  }
    }
    return nrOfValues > 0 ? sum/nrOfValues : 0;
}


void GsrSensor::loop()
{
    unsigned long currTime = millis();
    if ( currTime < _currReadTime) // we had a time reset
    {
        _currReadTime = currTime;
    } else if ( currTime - _currReadTime >= READ_SENSOR_INTERVAL ) {

        _currReadTime = currTime;
        _processSensor();
    }

    if ( currTime < _currReadSecondsTime) // we had a time reset
    {
    	_currReadSecondsTime = currTime;
    } else if ( currTime - _currReadSecondsTime >= 1000 ) {

        _currReadSecondsTime = currTime;
        _processSensorForSeconds();
    }    
}

void GsrSensor::_processSensor()
{
    _sensorValues[_sensorIndex] = analogRead(_pin);
    _sensorIndex += 1;
    if ( _sensorIndex == SENSOR_MAX_COUNT ) {
        _sensorIndex = 0;
		
	    _averageValue = _determineAverage();
   	 	if (_averageValue > _baseValue) {
        	_baseValue = _averageValue;
    	}    

		// keep adding to the per seconds array too
		_sensorSecondsValues[_sensorSecondsIndex] = _averageValue;
		_sensorSecondsIndex += 1;
		if ( _sensorSecondsIndex == SENSOR_MAX_PER_SECOND_COUNT ) {
			_sensorSecondsIndex = 0;
		}
    }
}

void GsrSensor::_processSensorForSeconds()
{
	// get average for last/current second
	int averageForSeconds = _determineAverageForSecond();
	_sensorLastSecondsValues[_sensorLastSecondsIndex] = averageForSeconds;

	_sensorLastSecondsIndex += 1;
	if ( _sensorLastSecondsIndex >= SENSOR_MAX_SECONDS_COUNT ) {
		_sensorLastSecondsIndex = 0;
	}

	// and clear the per seconds values
	//_resetPerSecondValues();
}

void GsrSensor::reset()
{
    int s1 = sizeof(_sensorValues) / sizeof(_sensorValues[0]);
    memset( _sensorValues, 0,  s1 * sizeof(_sensorValues[0]) );
    
    int s2 = sizeof(_sensorSecondsValues) / sizeof(_sensorSecondsValues[0]);
    memset( _sensorSecondsValues, 0,  s2 * sizeof(_sensorSecondsValues[0]) );
    
    int s3 = sizeof(_sensorLastSecondsValues) / sizeof(_sensorLastSecondsValues[0]);
    memset( _sensorLastSecondsValues, 0,  s3 * sizeof(_sensorLastSecondsValues[0]) );

	_resetPerSecondValues();

    _baseValue = 0;
    _averageValue = 0;
}

void GsrSensor::_resetPerSecondValues()
{
    int s1 = sizeof(_sensorSecondsValues) / sizeof(_sensorSecondsValues[0]);
    memset( _sensorSecondsValues, 0,  s1 * sizeof(_sensorSecondsValues[0]) );
}

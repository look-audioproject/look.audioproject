#ifndef SPIKE_H
#define SPIKE_H

#include <Arduino.h>
#include <SoftwareSerial.h>
#include "Utils.h"
#include "LookConstants.h"
#include "GsrSensor.h"

class Spike
{
public:
    Spike(byte track=1,PlayState state=FADEIN, byte fadeTime=FADE_TIME_IN_SECONDS, byte sensor=0, Emotion threshold=STRESSED,byte thresholdPercentage=THRESHOLD_SENSOR);
    bool loop(unsigned long playTime, SoftwareSerial &comms, int loopSensorRef1, int loopSensorRef2, int sensor1Average, int sensor2Average);
    void reset();
    bool isActiveInTimeRange(unsigned long playTime);
    bool isParsed();
    byte id;
    byte fadeTime;
    byte interval;
    byte repeat;
    byte sensor;
    byte compareWithSecondsAgo;
    PlayState state;
	TimeRange timeRange;
    Emotion threshold;
    byte thresholdDuration;
    byte thresholdPercentage;
    byte track;
    static int sensorReference1;
    static int sensorReference2;
private:
    String _processEligiblity(int base, int average);
    bool _sensorCrossesSpikeThreshold(int base, int average);
    byte _parsed;
    unsigned long _lastParsed;
};

/*
  byte thresholdDuration; // 1 default;  for how many seconds to measure the value (on average); max SENSOR_MAX_SECONDS_COUNT 
  byte thresholdPercentage; // the spike change
  byte repeat; // -1 infinite, > 0, the amount of times
  int track; // which track nr to fade
  long fadeTime; // time for fading in
  PlayState state; // FADEOUT for fade out, FADEIN for fade in
  byte user; // for which user do we need to check the sensor value (1 or 2 for now)
  Emotion threshold; // STRESSED for a lowering in sensor value, CALM for a highering in sensor value

  byte parsed; // nr of times it has been parsed, unless repeat is set to infinite, begins at 0
  long lastParsed; // last parse time, begins at 0
*/

#endif
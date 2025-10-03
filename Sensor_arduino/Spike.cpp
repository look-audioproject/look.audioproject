#include <Arduino.h>
#include <SoftwareSerial.h>
#include "GsrSensor.h"
#include "Utils.h"
#include "LookConstants.h"
#include "Spike.h"

int Spike::sensorReference1=-1;
int Spike::sensorReference2=-1;

Spike::Spike(byte track=1,PlayState state=FADEIN, byte fadeTime=FADE_TIME_IN_SECONDS, byte sensor=0, Emotion threshold=STRESSED,byte thresholdPercentage=THRESHOLD_SENSOR){
    thresholdDuration=0;
    this->thresholdPercentage=thresholdPercentage;
    repeat=-1;
    id=0;
    compareWithSecondsAgo=0;//use current
    this->track=track;
    this->fadeTime=fadeTime;
    interval=SPIKE_INTERVAL;
    this->state=state;
    this->sensor=sensor;
    timeRange = {0,0};
    this->threshold=threshold;
    
    _parsed=0;
    _lastParsed=-1*interval*1000;
}

bool Spike::isActiveInTimeRange(unsigned long playTime)
{
    return timeRange.until > 0 ? ((unsigned long)timeRange.start*1000 <= playTime && playTime < (unsigned long)timeRange.until*1000) : true;
}

bool Spike::isParsed()
{
    return repeat != -1 && _parsed >= repeat;
}

bool Spike::loop(unsigned long playTime, SoftwareSerial &comms, int loopSensorRef1, int loopSensorRef2, int sensor1Average, int sensor2Average)
{
    unsigned long currTime = millis();
    bool triggered = false;

    if ( _lastParsed > currTime )
    {
        _lastParsed = currTime;
    }

    if ( ( (sensor == 1 && loopSensorRef1 > 0) || (sensor == 2 && loopSensorRef2 > 0) ) && currTime > _lastParsed+(interval*1000) && isActiveInTimeRange(playTime) )
    {
        String cmd = _processEligiblity( sensor == 1 ? loopSensorRef1 : loopSensorRef2,
          sensor == 1 ? sensor1Average : sensor2Average );

        if ( cmd.length() > 0 )
        {
            triggered = true;
            comms.println(cmd);
        }
    }
    return triggered;
}

String Spike::_processEligiblity(int base, int average)
{
    String cmd = "";
    
    if ( _sensorCrossesSpikeThreshold(base, average) ) {

        if ( compareWithSecondsAgo <= 0 )
        {
            // UPDATE THE REFERENCE VALUE FOR THE NEXT LOOP ITERATION FOR ALL ITEMS FOR THIS SENSOR
            if ( sensor == 1 ) {
                // Serial.print("Updating reference for sensor 1: ");
                // Serial.println( average );
                Spike::sensorReference1 = average;
            } else {
                // Serial.print("Updating reference for sensor 2: ");
                // Serial.println( average );      
                Spike::sensorReference2 = average;
            }
        }
    
      cmd = "play";
      if ( state == FADEOUT ) {
        cmd = "stop";
      }
      
      _lastParsed = millis();
      _parsed += 1;

      cmd = cmd +":"+ track +"-"+ fadeTime;

      // Serial.print("SPIKE:: Used threshold ");
      // Serial.print(thresholdPercentage);
      // Serial.print(", state: ");
      // Serial.print(threshold);
      // Serial.print(" Checked spike for track ");
      // Serial.print(track);
      Serial.print(" decision for sensor ");
      Serial.print(sensor);
      // Serial.print(" parsed now: ");
      // Serial.print(_parsed);
      // Serial.print(" with base: ");
      // Serial.print(base);
      // Serial.print(" and curr: ");
      // Serial.println(average);     
    }

    return cmd;
}

void Spike::reset()
{
    _parsed = 0;
    _lastParsed = -1*interval*1000; // to start checking immediately
}

bool Spike::_sensorCrossesSpikeThreshold(int base, int average ) {
    float perc = (abs(base-average)/(base*1.0))*100.0;

    // Check if we have the right kind of up/down
    if ( (threshold == STRESSED && average < base) || ( threshold == RELAXED && average > base ) ) {

        // Serial.print("checkin spike ");
        // Serial.print(threshold);
        // Serial.print(" perc ");
        // Serial.print(perc);
        //   Serial.print(" base ");
        //   Serial.print(base);
        //   Serial.print(" average ");
        //   Serial.print(average);  
        //   Serial.print(" diff ");
        //   Serial.println(abs(base-average));

        bool crossed = perc >= thresholdPercentage;
        if ( crossed) {
            Serial.print("Spike ");
            Serial.print(id);
            Serial.print(" crossed percentage ");
            Serial.print(perc);
            Serial.print("; mood ");
            Serial.print((char)threshold);
            if ( interval > 0 )
            {
                Serial.print("; next check possible after ");
                Serial.print(interval);
                Serial.println(" seconds.");
            }
            else
            {
                Serial.println("");
            }
        }
        return crossed;
    }
    return false;
}

/*
 *
*struct SpikeDecision {
  byte thresholdDuration; // 1 default;  for how many seconds to measure the value (on average); max SENSOR_MAX_SECONDS_COUNT 
  byte thresholdPercentage; // the spike change
  byte repeat; // -1 infinite, > 0, the amount of times
  int track; // which track nr to fade
  long fadeTime; // time for fading in
  PlayState state; // FADEOUT for fade out, FADEIN for fade in
  byte sensor; // for which sensor do we need to check the sensor value (1 or 2 for now)
  Emotion threshold; // STRESSED for a lowering in sensor value, CALM for a highering in sensor value

  byte parsed; // nr of times it has been parsed, unless repeat is set to infinite, begins at 0
  long lastParsed; // last parse time, begins at 0
};
 *
*/
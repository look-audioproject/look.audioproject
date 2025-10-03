#include <SoftwareSerial.h>
#include "Utils.h"
#include "LookConstants.h"
#include "GsrSensor.h"

const bool DEBUG=false; // set to true to output some extra average value data each second per user

const int GSR1=A0;
const int GSR2=A2;

const byte TC1_TRANSMIT_PIN = 11; // normaal 11. send data to the track controller (connect there with the READ_PIN)
const byte TC1_READ_PIN = 10; // unused and unconnected
const byte TC2_TRANSMIT_PIN = 5; // send data to the track controller (connect there with the READ_PIN)
const byte TC2_READ_PIN = 7; // unused and unconnected

const byte LED1_TRANSMIT_PIN = 6; // send data to the LED controller (connect there with the READ_PIN of the Leds.ino)
const byte LED1_READ_PIN = 12;// Unused and unconnected
const byte LED2_TRANSMIT_PIN = 13; // send data to the LED controller (connect there with the READ_PIN of the Leds.ino)
const byte LED2_READ_PIN = 9;// Unused and unconnected

const byte STATUS_PIN = 4;
const byte BUTTON_PIN = 2;

const int PAUSE_TRIGGERS_AFTER_SPIKE = 9; // In seconds; after a spike pause all triggers for this users for X seconds
const int COMPARE_WITH_SECONDS_AGO = 8; // In seconds; compare the current value for a user with the average of the last X seconds										 // excludes the value of now
const int THRESHOLD_STRESSED_PERCENTAGE = 2; // Threshold for both users; if the difference crosses this percentage they're stressed
const int THRESHOLD_RELAXED_PERCENTAGE = 3; // Threshold for both users; if the difference crosses this percentage they're relaxed
const int FADE_TRACKS_IN_SECONDS = 2; // The fade time for tracks, both fading in and out

const long MAX_PLAY_TIME = 611; // play time in seconds

unsigned long currPlayTime=0;
unsigned int currPlayTimeInSeconds=0;
unsigned long currPlayStartTime=0;

int pauseTriggers1Until = -1;
int pauseTriggers2Until = -1;

bool firstStart = true;


bool playing = false; // false = ready to run; true = running;

byte buttonState = LOW;

SoftwareSerial trackController1 =  SoftwareSerial(TC1_READ_PIN, TC1_TRANSMIT_PIN);
SoftwareSerial trackController2 =  SoftwareSerial(TC2_READ_PIN, TC2_TRANSMIT_PIN);
SoftwareSerial ledController1 =  SoftwareSerial(LED1_READ_PIN, LED1_TRANSMIT_PIN);
SoftwareSerial ledController2 =  SoftwareSerial(LED2_READ_PIN, LED2_TRANSMIT_PIN);

GsrSensor sensor1(GSR1);
GsrSensor sensor2(GSR2);

void setup(){
  trackController1.begin(9600);
  trackController2.begin(9600);
  ledController1.begin(4800);
  ledController2.begin(4800);
  Serial.begin(19200);

  Serial.println("");
  Serial.println("LOOK - Sensor Controller");


  pinMode(TC1_TRANSMIT_PIN, OUTPUT);
  pinMode(TC2_TRANSMIT_PIN, OUTPUT);  
  pinMode(LED1_TRANSMIT_PIN, OUTPUT);
  pinMode(LED2_TRANSMIT_PIN, OUTPUT);
  pinMode(STATUS_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);

  if (!playing)
  {
    digitalWrite(STATUS_PIN, LOW);
  }
}

void loop(){

  if (firstStart) {
    firstStart = false;
    delay(2500);
    resetExperience();
    beginExperience();
  }

  if ( playing )
  {
    loopForSensorValues();
    loopForPlayTime();
    if ( currPlayTime >= 50) {
      loopForSpikes();
    }
  }

loopForStateButton();
}

bool areTriggersPausedFor1()
{
	return ( pauseTriggers1Until > -1 && currPlayTimeInSeconds < pauseTriggers1Until );
}

bool areTriggersPausedFor2()
{
	return ( pauseTriggers2Until > -1 && currPlayTimeInSeconds < pauseTriggers2Until );
}

void beginExperience(){

  Serial.println("Starting Experience");

  sensor1.reset();
  sensor2.reset();

  trackController1.println("start");
  trackController2.println("start");
  currPlayTime = 0;
  currPlayTimeInSeconds=0;
  currPlayStartTime = millis();
  pauseTriggers1Until = -1;
  pauseTriggers2Until = -1;

  playing = true;
  digitalWrite(STATUS_PIN, HIGH);

  // get a first value reading and update the leds
  loopForSensorValues();
  updateLedBaselines();
  ledController1.println("start");
  ledController2.println("start");  
}

void loopForPlayTime(){
  if ( playing ) {
    long currTime = millis();
    if ( currTime < currPlayTime ) { // we had a time reset
      currPlayStartTime = currTime;
    } else {
      currPlayTime = currTime-currPlayStartTime;  

	    unsigned int playTimeInSeconds = currPlayTime/1000;
      if ( currPlayTimeInSeconds != playTimeInSeconds ) {

        currPlayTimeInSeconds=playTimeInSeconds;
        Serial.print("PlayTime: ");
        Serial.println( playTimeInSeconds );       

		if ( pauseTriggers1Until > -1 && currPlayTimeInSeconds >= pauseTriggers1Until ) {
			pauseTriggers1Until = -1;
			Serial.println("Unpaused sensor 1");  
		}
		if ( pauseTriggers2Until > -1 && currPlayTimeInSeconds >= pauseTriggers2Until ) {
			pauseTriggers2Until = -1;
			Serial.println("Unpaused sensor 2");
		}

		if (DEBUG) {
			int avgOverLastSeconds1 = sensor1.averageValueForLastSeconds(COMPARE_WITH_SECONDS_AGO, true);
			Serial.print("avg user 1: ");
			Serial.println( avgOverLastSeconds1 );

			int avgOverLastSeconds2 = sensor2.averageValueForLastSeconds(COMPARE_WITH_SECONDS_AGO, true);
			Serial.print("avg user 2: ");
			Serial.println( avgOverLastSeconds2 );
		}

        // and update the sensor average for leds
		updateLeds();
      }
    }

    if (currPlayTime >= MAX_PLAY_TIME*1000) {
      resetExperience();
    }    
  }
}

void loopForSensorValues(){ 

  sensor1.loop();
  sensor2.loop();
  
  // Serial.print(" average user 1 ");
  // Serial.println(sensor1.averageValue());  
}

void loopForSpikes(){
  if (playing && currPlayTime > 50)
  {
	String cmd="";

	/// USER 1
	if ( ! areTriggersPausedFor1() ) {	
		int averageValueInLastXSeconds1 = sensor1.averageValueForLastSeconds( COMPARE_WITH_SECONDS_AGO );
		int averageValue1 = sensor1.averageValueForLastSecond();
		if ( sensorCrossesThreshold(averageValueInLastXSeconds1, averageValue1, STRESSED, THRESHOLD_STRESSED_PERCENTAGE, 1 ) ) {
			
			// Pause all triggers for this user for a couple of seconds
			pauseTriggers1Until = currPlayTimeInSeconds + PAUSE_TRIGGERS_AFTER_SPIKE;			

			// User 1 spiked to stressed, fade in track 2, fade out track 3
			cmd = "inout:2_3-"+ String(FADE_TRACKS_IN_SECONDS);
			trackController1.println(cmd);

      delay(10);

			cmd = "inout:4_5-"+ String(FADE_TRACKS_IN_SECONDS);
			trackController1.println(cmd);

    cmd = "play:5-"+ String(FADE_TRACKS_IN_SECONDS);
    trackController1.println(cmd);

		} else if ( sensorCrossesThreshold(averageValueInLastXSeconds1, averageValue1, RELAXED, THRESHOLD_RELAXED_PERCENTAGE, 1 ) ) {
			
			// Pause all triggers for this user for a couple of seconds
			pauseTriggers1Until = currPlayTimeInSeconds + PAUSE_TRIGGERS_AFTER_SPIKE;

			// User 1 spiked to relaxed, fade in track 3, fade out track 2
			cmd = "inout:3_2-"+ String(FADE_TRACKS_IN_SECONDS);
			trackController1.println(cmd);

      delay(10);
      
      cmd = "inout:5_4-"+ String(FADE_TRACKS_IN_SECONDS);
			trackController1.println(cmd);

      cmd = "stop:5-"+ String(FADE_TRACKS_IN_SECONDS);
      trackController1.println(cmd);

		}
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////////
 	
	cmd="";
	/// USER 2
	if ( ! areTriggersPausedFor2() ) {	
		int averageValueInLastXSeconds2 = sensor2.averageValueForLastSeconds( COMPARE_WITH_SECONDS_AGO );
		int averageValue2 = sensor2.averageValueForLastSecond();
		if ( sensorCrossesThreshold(averageValueInLastXSeconds2, averageValue2, STRESSED, THRESHOLD_STRESSED_PERCENTAGE, 2 ) ) {
			
			// Pause all triggers for this user for a couple of seconds
			pauseTriggers2Until = currPlayTimeInSeconds + PAUSE_TRIGGERS_AFTER_SPIKE;

			// User 2 spiked to stressed, fade in track 2, fade out track 3
			cmd = "inout:2_3-"+ String(FADE_TRACKS_IN_SECONDS);
			trackController2.println(cmd);

      delay(10);

			cmd = "inout:5_4-"+ String(FADE_TRACKS_IN_SECONDS);
			trackController2.println(cmd);

    cmd = "play:5-"+ String(FADE_TRACKS_IN_SECONDS);
    trackController2.println(cmd);

		} else if ( sensorCrossesThreshold(averageValueInLastXSeconds2, averageValue2, RELAXED, THRESHOLD_RELAXED_PERCENTAGE, 2 ) ) {
			
			// Pause all triggers for this user for a couple of seconds
			pauseTriggers2Until = currPlayTimeInSeconds + PAUSE_TRIGGERS_AFTER_SPIKE;

			// User 2 spiked to relaxed, fade in track 3, fade out track 2
			cmd = "inout:3_2-"+ String(FADE_TRACKS_IN_SECONDS);
			trackController2.println(cmd);

      delay(10);

			cmd = "inout:4_5-"+ String(FADE_TRACKS_IN_SECONDS);
			trackController2.println(cmd);

      cmd = "stop:5-"+ String(FADE_TRACKS_IN_SECONDS);
      trackController2.println(cmd);
		}
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////////

  }
}


void loopForStateButton(){
  byte newButtonState = digitalRead(BUTTON_PIN);
  if (buttonState == HIGH && newButtonState == LOW) {
    Serial.println("Pressed & Released");
    if ( playing ) 
    {
      resetExperience();
    }else{
      beginExperience();
    }
  }
  buttonState = newButtonState;
}

void updateLedBaselines()
{
  String update1 = "ref:1-";
  int value1 = sensor1.baseValue();
  String total1 = update1 + value1;
  ledController1.println(total1);

  String update2 = "ref:1-";
  int value2 = sensor2.baseValue();
  String total2 = update2 + value2;
  ledController2.println(total2);  
}

void updateLeds()
{
  if ( ! areTriggersPausedFor1() ) {
    String update1 = "avg:1-";
    int value1 = sensor1.averageValue();
    String total1 = update1 + value1;
    ledController1.println(total1);
  }

  if ( ! areTriggersPausedFor2() ) {
    String update2 = "avg:1-";
    int value2 = sensor2.averageValue();
    String total2 = update2 + value2;
    ledController2.println(total2);  
  }
}

void resetExperience(){

  Serial.println("Resetting Experience");
  playing = false;
  digitalWrite(STATUS_PIN, LOW);
  
  trackController1.println("reset");
  trackController2.println("reset");
  ledController1.println("reset");
  ledController2.println("reset");
}

bool sensorCrossesThreshold(int base, int average, Emotion threshold, byte thresholdPercentage, byte id ) {
    float perc = (abs(base-average)/(base*1.0))*100.0;

    // Check if we have the right kind of up/down
    if ( (threshold == STRESSED && average < base) || ( threshold == RELAXED && average > base ) ) {
        
        bool crossed = perc >= thresholdPercentage;
        if ( crossed) {
            Serial.print("Spike ");
            Serial.print(id);
            Serial.print(" crossed percentage ");
            Serial.print(perc);
            Serial.print("; mood ");
            Serial.print((char)threshold);
            Serial.println("");
        }
        return crossed;
    }
    return false;
}
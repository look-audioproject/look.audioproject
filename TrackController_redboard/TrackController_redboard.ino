

#include <SparkFun_Tsunami_Qwiic.h> //http://librarymanager/All#SparkFun_Tsunami_Super_WAV_Trigger
#include <SoftwareSerial.h>

TsunamiQwiic tsunami;

const byte FADE_TIME_IN_SECONDS = 3;
const byte READ_PIN = 10;
const byte TRANSMIT_PIN = 12;

const int DECIBEL_OFF = -70; // an off sound
const int DECIBEL_ON = 20; // an on sound

bool tsunamiReady = true;

String command = "";

SoftwareSerial fromSensors =  SoftwareSerial(READ_PIN, TRANSMIT_PIN);

void setup() {

  pinMode(READ_PIN, INPUT);
  pinMode(TRANSMIT_PIN, OUTPUT);
    
  fromSensors.begin(9600);
  Serial.begin(57600);

  Serial.println("");
  Serial.println("LOOK - Tsunami Controller");

  Wire.begin();

  if (tsunami.begin() == false)
  {
    tsunamiReady = false;
    Serial.println("Tsunami Qwiic failed to respond.");    
  };     

  stopAllTracksNow();
  Serial.println("LOOK - Tsunami Controller - Done setting up");
}

void fadeInTrack(int which, long seconds) {
  tsunami.trackFade(which, DECIBEL_ON, 1000*seconds, false);
}

void fadeOutTrack(int which, long seconds) {
   tsunami.trackFade(which, DECIBEL_OFF, 1000*seconds, false);
}

void loop() {

    if (fromSensors.available() > 0) {
      int data = fromSensors.read();
      if ( data != 13 && data != 10 ) {
        char character = data;
        command += character;
      }
      else if (data == 10)
      {
        Serial.println(command +" received");
        // parse command
        parseCommand(command); // so it can be changed again in between parsing time, if that would ever happen.
        command = "";
      }       
    }
}

void start()
{
  stopAllTracksNow();

  /*
    tsunami.trackLoad(a, b);
    a = track nummer
    b = output nummer
  */

  tsunami.trackLoad(1, 1); // voor gebruiker 1
  tsunami.trackGain(1, 20);
  
  tsunami.trackLoad(2, 1); // for STRESS
  tsunami.trackGain(2, -70);

  tsunami.trackLoad(3, 1); // for RELAXED
  tsunami.trackGain(3, -70);

  tsunami.trackLoad(4, 0); // MUSIC
  tsunami.trackGain(4, 10);

  tsunami.trackLoad(5, 0); // MUSIC for STRESS
  tsunami.trackGain(5, -70);


  // Start playing them all at the same time.
  tsunami.resumeAllInSync();
}

void parseCommand(String cmd)
{
  int sepIdx = cmd.indexOf(":");
  if ( sepIdx >= 0 ) {

    String action = cmd.substring(0,sepIdx);
    String data = cmd.substring(sepIdx+1);

    int fadeIdx = data.indexOf("-");
    long fadeTime = FADE_TIME_IN_SECONDS;
    if ( fadeIdx > 0 ) {
      fadeTime = data.substring(fadeIdx+1).toFloat();
      Serial.print("Fade: ");
      Serial.print(fadeTime);
      Serial.print(", ");
      data = data.substring(0,fadeIdx);
    }

    if ( action == "play" ) {
        Serial.print("Playing track ");
        Serial.println( data );
        fadeInTrack( data.toInt(), fadeTime );
    } else if ( action == "stop" ) {
        Serial.print("Stopping track");
        Serial.println( data );
        fadeOutTrack( data.toInt(), fadeTime );
    } else if (action == "inout") {

      int trackSep = data.indexOf("_");  
      if ( trackSep > 0) {
        int trackToPlay = data.substring(0,trackSep).toInt();
        int trackToStop = data.substring(trackSep+1).toInt();

        fadeInTrack( trackToPlay, fadeTime );
        fadeOutTrack( trackToStop, fadeTime );   

        Serial.print("Playing track ");
        Serial.print(trackToPlay);
        Serial.print(", stopping track ");
        Serial.println(trackToStop);        

      } else {
        Serial.println("Wrong 'inout' input data");
      }

    } else {
      Serial.println("");
    }

  } else {
    if ( cmd == "reset" ) {
        Serial.println("Resetting");
        reset();
    } else if ( cmd == "start") {
        Serial.println("Starting experience");
        start();
    }
  }
}

void playTrack(int which) {
  tsunami.trackPlayPoly(which, 0);
}

void reset()
{
  stopAllTracksNow();
}

void stopTrackNow(int which) {
  tsunami.trackStop(which);
}

void stopAllTracksNow() {
  tsunami.stopAllTracks();
}


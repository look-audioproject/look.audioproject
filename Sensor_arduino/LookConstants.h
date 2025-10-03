#ifndef LOOKCONSTANTS_H
#define LOOKCONSTANTS_H

enum Emotion {RELAXED='R', STRESSED='S'};
enum PlayState {FADEIN,FADEOUT};

// DEFAULT: above this percentage value it's stress, below it's relaxed
const byte THRESHOLD_SENSOR = 10; // percentage 0 - 100

// DEFAULT: Time for a track to fade in/out
const byte FADE_TIME_IN_SECONDS = 4; // in seconds

// DEFAULT: once a spike has been registered, don't execute the decision again 
// before this nr of seconds
const byte SPIKE_INTERVAL = 1; // in seconds 

#endif
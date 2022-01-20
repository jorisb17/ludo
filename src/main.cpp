#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_VS1053.h>
#include <SD.h>
#include <stdio.h>
#include <stdlib.h>
#include <Vector.h>
#include <assert.h>
#include <time.h>

// These are the pins used for the music maker shield
#define SHIELD_RESET  -1      // VS1053 reset pin (unused!)
#define SHIELD_CS     7      // VS1053 chip select pin (output)
#define SHIELD_DCS    6      // VS1053 Data/command select pin (output)

// These are common pins between breakout and shield
#define CARDCS 4     // Card chip select pin
// DREQ should be an Int pin, see http://arduino.cc/en/Reference/attachInterrupt
#define DREQ 3       // VS1053 Data request, ideally an Interrupt pin

#define COIN_INPUT 8

enum State {IDLE, PLAYING, SELECT_NORMAL, SELECT_ATTENTION};

State state = IDLE;

volatile int timer = 0;
bool playAttention = false;

Adafruit_VS1053_FilePlayer musicPlayer = 
  // create shield-example object!
  Adafruit_VS1053_FilePlayer(SHIELD_RESET, SHIELD_CS, SHIELD_DCS, DREQ, CARDCS);

String oVoiceArray[3];
Vector<String> oVoiceList(oVoiceArray);

String oRandomArray[3];
Vector<String> oRandomList(oRandomArray);

void printDirectory(File dir, int numTabs);
int freeMemory();
  
void setup() {
  Serial.begin(9600);
  Serial.println("Adafruit VS1053 Simple Test");

  if (! musicPlayer.begin()) { // initialise the music player
     Serial.println(F("Couldn't find VS1053, do you have the right pins defined?"));
     while (1);
  }
  Serial.println(F("VS1053 found"));
  
   if (!SD.begin(CARDCS)) {
    Serial.println(F("SD failed, or not present"));
    while (1);  // don't do anything more
  }

  File dir = SD.open("/v/");
  while(true) {
     
    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      //Serial.println("**nomorefiles**");
      break;
    }
    
    if(freeMemory() <= 21){
      break;
    }

    oVoiceList.push_back(entry.name());
    
    if (entry.isDirectory()) {
      continue;
    }

    entry.close();
  }

  dir = SD.open("/r/");
  while(true) {
     
    File entry =  dir.openNextFile();
    if (! entry) {
      break;
    }
    
    if(freeMemory() < 21){
      break;  
    }

    oRandomList.push_back(entry.name());
    
    if (entry.isDirectory()) {
      continue;
    }

    entry.close();
  }

  // Set volume for left, right channels. lower numbers == louder volume!
  musicPlayer.setVolume(10,10);

  // If DREQ is on an interrupt pin (on uno, #2 or #3) we can do background
  // audio playing
  musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);  // DREQ int

  pinMode(COIN_INPUT, INPUT_PULLUP);

  srand(time(NULL));

  //set timer 1 interrupt a 1 Hz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = 15624;// = (16*10^6) / (1*1024) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS10 and CS12 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);

  sei();
}

String prefix;
String name;
int r;

void loop() {
  switch (state)
  {
  case IDLE:
    if(digitalRead(COIN_INPUT) == LOW)
      state = SELECT_NORMAL;
    else if(playAttention)
      state = SELECT_ATTENTION;
    break;
  case SELECT_ATTENTION:
    r = rand() % oRandomList.size();
    name = oRandomList.at(r);
    prefix = "/r/";
    prefix += name;
    musicPlayer.startPlayingFile(prefix.c_str());
    state = PLAYING;
    break;
  case SELECT_NORMAL:
    r = rand() % oVoiceList.size();
    name = oVoiceList.at(r);
    prefix = "/v/";
    prefix += name;
    musicPlayer.startPlayingFile(prefix.c_str());
    state = PLAYING;
    break;
  case PLAYING:
    if(!musicPlayer.playingMusic)
      state = IDLE;
    
    //TODO implement Ludo's code
    break;
  default:
    break;
  }
}

//
// Used in calculating free memory.
//
extern unsigned int __bss_end;
extern void *__brkval;

//
// Returns the current amount of free memory in bytes.
//
int freeMemory() {
	int free_memory;
	if ((int) __brkval)
		return ((int) &free_memory) - ((int) __brkval);
	return ((int) &free_memory) - ((int) &__bss_end);
}

ISR(TIMER1_COMPA_vect){
  if(state == IDLE){
    timer++;
    if(timer == 300){
      playAttention = true;
      timer = 0;
    }
  }else{
    timer = 0;
  }
}



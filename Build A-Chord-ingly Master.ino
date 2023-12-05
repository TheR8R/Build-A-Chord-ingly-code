#include "Arduino.h" // Library for standard definitions and functions
#include "SoftwareSerial.h" // Library for serial communication with DFPlayer Mini
#include "DFRobotDFPlayerMini.h" // Library for controlling the DFPlayer Mini
#include "Wire.h" // Library for I2C communication


// Use pins 2 and 3 to communicate with DFPlayer Mini
static const uint8_t PIN_MP3_TX = 2; // Connects to module's RX 
static const uint8_t PIN_MP3_RX = 3; // Connects to module's TX 
SoftwareSerial softwareSerial(PIN_MP3_RX, PIN_MP3_TX);

const int startButtonPin = 4; //start Button pin
const int checkButtonPin = 9; //check Button pin

int startButtonState = 0; //variable for controlling state of start button
int checkButtonState = 0; //variable for controlling state of check button

// Create the Player object
DFRobotDFPlayerMini player;

float responseFrom1 = 0; //Response from first slave
float responseFrom2 = 0; //Response from second slave
float responseFromSelf = 0;

float lastReadNote = 0;
float currentReadNote = 0; // variable to store the data for that specific RF ID to then match on what block it is

// Definition of our tones identifiers
float A = 730; // A block identifier
float B = 714; // B block identifier
float C = 734; // C block identifier
float D = 667; // D block identifier
float E = 742; // E block identifier 
float F = 682; // F block identifier 
float G = 680; // G block identifier

// data structure for chords
struct Chord {
  float tone1;
  float tone2;
  float tone3;
};


// Chord definitions
Chord CChord = {C, E, G}; //C Chord
Chord DChord = {D, F, A}; //D Chord
Chord EChord = {E, G, B}; //E Chord
Chord FChord = {F, A, C}; //F Chord
Chord GChord = {G, B, D}; //G Chord
Chord AChord = {A, C, E}; //A Chord
Chord BChord = {B, D, F}; //B Chord
Chord NoChord = {0, 0, 0}; //No Chord

Chord chords[] = {AChord, BChord, CChord, DChord, EChord, FChord, GChord};


Chord currentChord = NoChord;
int index = 0; // index for keeping track of which chord we have reached, so we don't go out of bound

void setup() {
  Wire.begin(); //I2C connectionsetup master. No paramaeter indicates this is the master
  Serial.begin(9600); // connect to the serial port
  softwareSerial.begin(9600);// Init serial port for DFPlayer Mini

  if (player.begin(softwareSerial, true, false)) {
    Serial.println("OK");
    // Set volume to maximum (0 to 30).
    player.volume(15);
  } else {
    Serial.println("Connecting to DFPlayer Mini failed!");
  }
  pinMode(startButtonPin, INPUT); //initialize the pushbutton pin
  pinMode(checkButtonPin, INPUT); //initialize the checkbutton pin
}

// Function to convert Chord struct to a char array
char* chordToStr(Chord chord) {
  char* buffer = (char*)malloc(sizeof(Chord)); // Allocate memory for the char array
  memcpy(buffer, &chord, sizeof(Chord)); // Copy the Chord struct to the char array
  return buffer; // Return the char array
}

void sendChordToSlave(int slaveAddr, Chord chord) {
  char* dataToSend = chordToStr(chord); // Convert the Chord struct to a char array
  Wire.beginTransmission(slaveAddr); // Address of the slave device to send to
  Wire.write(dataToSend, sizeof(Chord)); // Send char array to slave
  Wire.endTransmission();
  free(dataToSend); // Free allocated memory
}

float receiveNoteFromSlave(int slaveAddr) {
  Wire.requestFrom(slaveAddr, 1); //Request 1 bytes from slave 
  String dataString = "";  // create a string for accumulation
  while (Wire.available()) { //while we are recieving stuff from slave
    char c = Wire.read(); //read what we got
    dataString = dataString + c; //accumulate it
  }
  float tmp = dataString.toFloat(); //turn the data into a float
  return tmp; //return it
}

/* 
  Plays the sound file explaining the name of the given chord
*/
void playNameOfChord (Chord chord){
  float tone = chord.tone1;
  if(tone == A){player.play(1);}
  if(tone == B){player.play(2);}
  if(tone == C){player.play(3);}
  if(tone == D){player.play(4);}
  if(tone == E){player.play(5);}
  if(tone == F){player.play(6);}
  if(tone == G){player.play(7);}
}

/* 
  Plays the sound file for the given chord
*/
void playCurrentChord (Chord chord){
  float tone = chord.tone1;
  if(tone == A){player.playFolder(1, 1);}
  if(tone == B){player.playFolder(1, 2);}
  if(tone == C){player.playFolder(1, 3);}
  if(tone == D){player.playFolder(1, 4);}
  if(tone == E){player.playFolder(1, 5);}
  if(tone == F){player.playFolder(1, 6);}
  if(tone == G){player.playFolder(1, 7);}
}


/* 
  Function for listening to button states,
  and performs the actions, depending on which state the buttons are in
*/
void listen () {
  lastReadNote = 0; //We need to reset this so blocks can be placed again

  if(checkButtonState == LOW){ //if have not failed checking for the current chord
    currentChord = chords[index]; //get the chord we are trying to make
    index += 1; //increment the index, such that next time we can get the next chord
    if(index == 7) { //if we have reached this point, we are out of chords and have to start over
      index = 0;
    }
  }
  //Serial.println(currentChord.tone1);
  playNameOfChord(currentChord);
  //transmit the selected chord to the slaves
  sendChordToSlave(1, currentChord);
  //Serial.println("I sent stuff to slave 1");
  sendChordToSlave(2, currentChord);
  //Serial.println("I sent stuff to slave 2");

  startButtonState = digitalRead(startButtonPin); // Needed for making a new chord
  checkButtonState = digitalRead(checkButtonPin); // This is needed for the try again with check button
  while(checkButtonState == LOW){
    responseFrom1 = receiveNoteFromSlave(1);
    responseFrom2 = receiveNoteFromSlave(2);

       //the master listens to his own RFID reader.
    while(Serial.available() > 0) {   //as long as we can read something, then do it
      currentReadNote += Serial.read(); // Accumulate the info of the tag read by the rfid reader
    }

    if(currentReadNote != 0){ //if we have read something
      lastReadNote = currentReadNote; //then assign it to lastReadNote
    }
    currentReadNote = 0; //reset current read note, to allow for other note-blocks to be read
    bool noteIsPartOfCurrentChord = (currentChord.tone1 == lastReadNote || currentChord.tone2 == lastReadNote || currentChord.tone3 == lastReadNote); //check if the note-block we just read is a part of the chord we are making
    if(noteIsPartOfCurrentChord) { //if yes
      responseFromSelf = 1;
    } else {  // if no
      responseFromSelf = 0; 
    }
    
    startButtonState = digitalRead(startButtonPin); //check if start button is pressed
    if (startButtonState == HIGH) { //check if the start button has been presed to allow cycling through the chords
      delay(300); // very important 300 milisecond delay, that ensures cycling through chords works as intended
      listen(); //start over the method, such that a new chord can be send to slaves
    } 
    
    checkButtonState = digitalRead(checkButtonPin); //check the checkbutton such that we can exit this method and go to loop
  }
}

void loop () {
  startButtonState = digitalRead(startButtonPin); //read state
  if (startButtonState == HIGH) {
    listen(); // start listening for the next chord
  } 

  if(currentChord.tone1 != 0){
    checkButtonState = digitalRead(checkButtonPin);
    if (checkButtonState == HIGH) {
      bool correctChordHasBeenProduced = (responseFrom1 == 1 && responseFromSelf == 1  && responseFrom2 == 1);
      if(correctChordHasBeenProduced) {
        playCurrentChord(currentChord); //play the current chords
      } else {
        // the chord was not produced correctly
        player.play(8); //wrong you failed
        delay(3000); //Delay to allow the failed sound to finish playing
        listen();  // start listening for the next chord
      }
    }
  }
}

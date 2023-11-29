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
  delay(1000); //without this delay the miniplayer wont work
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

Chord getChord() {
  int chordNumberForSlave = random(sizeof(chords));
  return chords[chordNumberForSlave];
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
  String dataString = "";
  while (Wire.available()) {
    char c = Wire.read();
    dataString = dataString + c;
  }
  float tmp = dataString.toFloat();
  delay(500);
  //Serial.println("i received stuff from slave " + slaveAddr);
  //Serial.println(tmp);
  return tmp;
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
  lastReadNote = 0;

  //Serial.println("button pressed");
  if(checkButtonState == LOW){
    currentChord = chords[index];
    index += 1;
    if(index == 7) {
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
    //Serial.println("I am in the loop GET ME OUT!");
    
    responseFrom1 = receiveNoteFromSlave(1);
    responseFrom2 = receiveNoteFromSlave(2);

    //the master listens to his own RFID reader.
    while(Serial.available() > 0) {
      currentReadNote += Serial.read(); // Accumulate the info of the tag read by the rfid reader
    }

    if(currentReadNote != 0){
      lastReadNote = currentReadNote;
    }
    currentReadNote = 0;
    //Serial.println(lastReadNote);
    bool noteIsPartOfCurrentChord = (currentChord.tone1 == lastReadNote || currentChord.tone2 == lastReadNote || currentChord.tone3 == lastReadNote);
    if(noteIsPartOfCurrentChord) {
      responseFromSelf = 1;
      //Serial.println(responseFromSelf);
    } else {
      responseFromSelf = 0;
    }
    startButtonState = digitalRead(startButtonPin);
    if (startButtonState == HIGH) {
      listen();
    } 

    checkButtonState = digitalRead(checkButtonPin);
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
      //Serial.println("ey this one works too and we stopped the loop");
      bool correctChordHasBeenProduced = (responseFrom1 == 1 && responseFromSelf == 1  && responseFrom2 == 1);
      if(correctChordHasBeenProduced) {
        //Serial.print("YES! all tags reported true and it worked!");
        playCurrentChord(currentChord); //play the current chords
      } else {
        // the chord was not produced correctly
        //Serial.print("yeh no, the things weren't read correctly");
        player.play(8); //wrong you failed
        delay(3000);
        listen();  // start listening for the next chord
      }
    }
  }
}


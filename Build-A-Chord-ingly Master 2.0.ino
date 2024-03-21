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
int modeToggle = 0; //variable for controlling the mode. 0 is listen, and 1 is sandbox.
int chordHasPlayed = 0; //flag variable for detecting if we have to play failed sound in sandboxmode.
int correctChord = 1; //flag variable to ensure that we don't skip to next chord in sandboxmode.

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
float Mode = 697; // mode identifier to switch mode

// data structure for chords
struct Chord {
  float tone1;
  float tone2;
  float tone3;
  float tone4;
};

// Chord definitions
Chord CChord = {C, E, G, 2156}; //C Chord
Chord DChord = {D, F, A, 2079}; //D Chord
Chord EChord = {E, G, B, 2136}; //E Chord
Chord FChord = {F, A, C, 2146}; //F Chord
Chord GChord = {G, B, D, 2061}; //G Chord
Chord AChord = {A, C, E, 2206}; //A Chord
Chord BChord = {B, D, F, 2063}; //B Chord
Chord NoChord = {0, 0, 0, 0}; //No Chord

Chord chords[] = {AChord, BChord, CChord, DChord, EChord, FChord, GChord};

//an array of random numbers for getting the chords in a seemingly random order in sandboxMode
int chordNumber[] = {2, 5, 1, 4, 0, 6, 3, 2, 1, 5, 3, 0, 4, 6, 1, 2, 5, 0, 3, 4, 6};

Chord currentChord = NoChord;
int index = 0; // index for keeping track of which chord we have reached, so we don't go out of bound
int sandboxIndex = 0;

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
  delay(85);
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
////////////////////////////////////////////////////
void assignValue(int slaveNumber, float blockNumber){
  if(slaveNumber == 1){  
    if(blockNumber == 1){responseFrom1 = 730;}
    if(blockNumber == 2){responseFrom1 = 714;}
    if(blockNumber == 3){responseFrom1 = 734;}
    if(blockNumber == 4){responseFrom1 = 667;}
    if(blockNumber == 5){responseFrom1 = 742;}
    if(blockNumber == 6){responseFrom1 = 682;}
    if(blockNumber == 7){responseFrom1 = 680;}}
  if(slaveNumber == 2){
    if(blockNumber == 1){responseFrom2 = 730;}
    if(blockNumber == 2){responseFrom2 = 714;}
    if(blockNumber == 3){responseFrom2 = 734;}
    if(blockNumber == 4){responseFrom2 = 667;}
    if(blockNumber == 5){responseFrom2 = 742;}
    if(blockNumber == 6){responseFrom2 = 682;}
    if(blockNumber == 7){responseFrom2 = 680;}}
}
///////////////////////////////////////////////////////
/* 
  Function for listening to button states,
  and performs the actions, depending on which state the buttons are in
*/
void listen () {
  Serial.println("we in listen");
  lastReadNote = 0; //We need to reset this so blocks can be placed again

  if(checkButtonState == LOW){ //if have not failed checking for the current chord
    currentChord = chords[index]; //get the chord we are trying to make
    index += 1; //increment the index, such that next time we can get the next chord
    if(index == 7) { //if we have reached this point, we are out of chords and have to start over
      index = 0;
    }
  }
  playNameOfChord(currentChord);
  //transmit the selected chord to the slaves
  sendChordToSlave(1, currentChord);
  sendChordToSlave(2, currentChord);

  startButtonState = digitalRead(startButtonPin); // Needed for making a new chord
  checkButtonState = digitalRead(checkButtonPin); // This is needed for the try again with check button
  while(checkButtonState == LOW){
    float recievedFrom1 = receiveNoteFromSlave(1);
    float recievedFrom2 = receiveNoteFromSlave(2);
    assignValue(1, recievedFrom1);
    assignValue(2, recievedFrom2);

    //the master listens to his own RFID reader.
    while(Serial.available() > 0) {   //as long as we can read something, then do it
      currentReadNote += Serial.read(); // Accumulate the info of the tag read by the rfid reader
    }

    if(currentReadNote != 0){ //if we have read something
      responseFromSelf = currentReadNote;
     // lastReadNote = currentReadNote; //then assign it to lastReadNote
    }
    currentReadNote = 0; //reset current read note, to allow for other note-blocks to be read
    //Swtich mode
    if(responseFromSelf == Mode){
        Serial.println("SWITCHING MODE");
      responseFromSelf = 0;
      modeToggle = 1;
      delay(80);
      break;
    }


    startButtonState = digitalRead(startButtonPin); //check if start button is pressed
    if (startButtonState == HIGH) { //check if the start button has been presed to allow cycling through the chords
      delay(300); // very important 300 milisecond delay, that ensures cycling through chords works as intended
      listen(); //start over the method, such that a new chord can be send to slaves
    } 
    
    checkButtonState = digitalRead(checkButtonPin); //check the checkbutton such that we can exit this method and go to loop
  }
    Serial.println("we out off loop in listen");
  if(modeToggle == 0){
      Serial.println("we gotta stay in listen");
    if(currentChord.tone1 != 0){
      checkButtonState = digitalRead(checkButtonPin);
      if (checkButtonState == HIGH) {
          Serial.println(responseFrom1);
          Serial.println(responseFrom2);
          Serial.println(responseFromSelf);
        float theChord = responseFrom1 + responseFromSelf + responseFrom2;
          Serial.println("the chord");
          Serial.println(theChord);
        bool correctChordHasBeenProduced = theChord == currentChord.tone4;
          Serial.println("correctchordhasbeenproduced");
          Serial.println(correctChordHasBeenProduced);
        if(correctChordHasBeenProduced) {
          playCurrentChord(currentChord); //play the current chords
          theChord = 0;
        } else {
          // the chord was not produced correctly
          player.play(8); //wrong you failed
          theChord = 0;
          delay(3000); //Delay to allow the failed sound to finish playing
          listen();  // start listening for the next chord
        }
      }
    }
  }
  Serial.println("we leaving listen");
}

void sandboxMode(){
  Serial.println("we in sandbox");
  lastReadNote = 0; //We need to reset this so blocks can be placed again
  if(correctChord == 1){
    responseFrom1 = 0;
    responseFrom2 = 0;
    responseFromSelf = 0;
    int chordIndex = chordNumber[sandboxIndex];
    if(sandboxIndex == 20){
      sandboxIndex = -1;
    }
    sandboxIndex += 1;
    player.play(14);
    delay(2000);
    currentChord = chords[chordIndex]; //get the chord we are trying to make
    correctChord = 0;
  }

  playCurrentChord(currentChord); //play the current chords
  //transmit the selected chord to the slaves
  sendChordToSlave(1, currentChord);
  sendChordToSlave(2, currentChord);

  startButtonState = digitalRead(startButtonPin); // Needed for playing the chord again
  checkButtonState = digitalRead(checkButtonPin); // This is needed for playing the produced chord
  while(checkButtonState == LOW){
    float recievedFrom1 = receiveNoteFromSlave(1);
    float recievedFrom2 = receiveNoteFromSlave(2);
    assignValue(1, recievedFrom1);
    assignValue(2, recievedFrom2);

    //the master listens to his own RFID reader.
    while(Serial.available() > 0) {   //as long as we can read something, then do it
      currentReadNote += Serial.read(); // Accumulate the info of the tag read by the rfid reader
    }

    if(currentReadNote != 0){ //if we have read something
      responseFromSelf = currentReadNote;
      // lastReadNote = currentReadNote; //then assign it to lastReadNote
    }
    currentReadNote = 0; //reset current read note, to allow for other note-blocks to be read
    //Swtich mode
    if(responseFromSelf == Mode){
      responseFromSelf = 0;
      modeToggle = 0;
      delay(80);
      break;
    }

    startButtonState = digitalRead(startButtonPin); //check if start button is pressed
    if (startButtonState == HIGH) { //check if the start button has been presed to allow cycling through the chords
      delay(300); // very important 300 milisecond delay, that ensures cycling through chords works as intended
      playCurrentChord(currentChord); //play the current chords
    } 

    checkButtonState = digitalRead(checkButtonPin); //check the checkbutton such that we can exit this method and go to loop
  }
  if(modeToggle == 1){
    if(currentChord.tone1 != 0){
      checkButtonState = digitalRead(checkButtonPin);
      if (checkButtonState == HIGH) {
        float theChord = responseFrom1 + responseFromSelf + responseFrom2;
          Serial.println("the chord");
          Serial.println(theChord);
          Serial.println("the CURRENTCHORDTONE4");
          Serial.println(currentChord.tone4);
          
        bool correctChordHasBeenProduced = theChord == currentChord.tone4;
          Serial.println("correctchordhasbeenproduced");
          Serial.println(correctChordHasBeenProduced);
        if(correctChordHasBeenProduced) {
          correctChord = 1;
          player.play(12);
          delay(2500);
        } else {
          for(int i = 0; i < 7; i++){
            Chord possibleChord = chords[i];
            if(possibleChord.tone4 == theChord){
              playCurrentChord(possibleChord);
              delay(3500);
              player.play(13);
              delay(2500);
              chordHasPlayed = 1;
            }
          }
          if(chordHasPlayed == 0){
            player.play(11); //wrong you failed
            delay(3000); //Delay to allow the failed sound to finish playing

          }
          chordHasPlayed = 0;
        }
        sandboxMode();
      }
    }
  }

}



void loop () {
  startButtonState = digitalRead(startButtonPin); // Needed for making a new chord
  checkButtonState = digitalRead(checkButtonPin); // This is needed for the try again with check button
  delay(500);
  if(modeToggle == 1){
    player.play(10); 
    delay(3000); //Delay to allow the failed sound to finish playing
    sandboxMode();
  }
  if(modeToggle == 0){
    player.play(9); 
    delay(3000); //Delay to allow the failed sound to finish playing
    listen();
  }
}

#include "Arduino.h" // Including the Arduino library for standard definitions and functions
#include "Wire.h" // Including the Wire library for I2C communication

/* 
    Datastructure to hold information about which tones
    the slave should look for.
*/
struct Chord {
  float tone1; 
  float tone2; 
  float tone3; 
  float tone4;
};

float lastReadNote = 0; // Variable to store the last noteblock(tag) read by the rfid reader
float currentReadNote = 0; // Variable to store the the value of the noteblock thats just been read.
char dataToSend = '0'; // Variable to store data that will be sent to the master

Chord receivedChord; // Variable to store the chord received from master

///////////////////////////////////////////////////////////////////////////////77
// Definition of our tones identifiers
float A = 730; // A block identifier
float B = 714; // B block identifier
float C = 734; // C block identifier
float D = 667; // D block identifier
float E = 742; // E block identifier 
float F = 682; // F block identifier 
float G = 680; // G block identifier
//////////////////////////////////////////////////////////////////////

void setup() {
  Wire.begin(2); // Initialize the I2C communication as a slave. REMEMBER to change the parameter value when writing to different slaves
  Wire.onReceive(receiveEvent); // Set up a function to be called when data is received by the master
  Wire.onRequest(requestEvent); // Set up a function to be called when data is requested by the master
  Serial.begin(9600); // Initialize the serial communication at 9600 baud rate
}

void loop() {
  while(Serial.available() > 0) {
    currentReadNote += Serial.read(); // Accumulate the info of the tag read by the rfid reader
  }
  delay(100);

  if(currentReadNote != 0){
    Serial.println("currentReadNote");
    Serial.println(currentReadNote);
    lastReadNote = currentReadNote;
    Serial.println("lastReadNote");
    Serial.println(lastReadNote);
    
      //////////////////////////////////////
    if(lastReadNote == A){dataToSend = '1';}
    if(lastReadNote == B){dataToSend = '2';}
    if(lastReadNote == C){dataToSend = '3';}
    if(lastReadNote == D){dataToSend = '4';}
    if(lastReadNote == E){dataToSend = '5';}
    if(lastReadNote == F){dataToSend = '6';}
    if(lastReadNote == G){dataToSend = '7';}
    ////////////////////////////////////////
    currentReadNote = 0; // Reset the current note so we inorder to read 
  }
  //Serial.println(lastReadNote);


    //Serial.println(dataToSend);
}

/* 
    Function to handle the data received from master
*/
void receiveEvent(int numBytes) {
  Serial.println("I recieved something");
  if (numBytes == sizeof(Chord)) { // Check if the received data matches the size of the Chord structure
    char* dataReceived = (char*)malloc(sizeof(Chord)); // Allocate memory for received data
    Wire.readBytes(dataReceived, sizeof(Chord)); // Read the data into the allocated memory
    memcpy(&receivedChord, dataReceived, sizeof(Chord)); // Convert char array to Chord struct
    free(dataReceived); // Free the allocated memory of dataReceived
    lastReadNote = 0; // Reset the last read note
    /*
    Serial.println("Received Chord -->");
    Serial.println("Tone 1: ");
    Serial.println(receivedChord.tone1);
    Serial.println("Tone 2: ");
    Serial.println(receivedChord.tone2);
    Serial.println("Tone 3: ");
    Serial.println(receivedChord.tone3); */
  }
}

// Function to handle the data that should be sent to the master upon request
void requestEvent() {
  Serial.println("datatoSend");
  Serial.println(dataToSend);
  Wire.write(dataToSend); // Send the data stored in dataToSend
}

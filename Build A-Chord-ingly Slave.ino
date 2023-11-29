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
};

float lastReadNote = 0; // Variable to store the last noteblock(tag) read by the rfid reader
float currentReadNote = 0; // Variable to store the the value of the noteblock thats just been read.
char dataToSend = '0'; // Variable to store data that will be sent to the master

Chord receivedChord; // Variable to store the chord received from master

void setup() {
  Wire.begin(2); // Initialize the I2C communication as a slave. REMEMBER to change the parameter value when writing to different slaves
  Wire.onReceive(receiveEvent); // Set up a function to be called when data is received by the master
  Wire.onRequest(requestEvent); // Set up a function to be called when data is requested by the master
  Serial.begin(9600); // Initialize the serial communication at 9600 baud rate
}

void loop() {
  delay(500); // Delay for 500 milliseconds
  while(Serial.available() > 0) {
    currentReadNote += Serial.read(); // Accumulate the info of the tag read by the rfid reader
    //Serial.println(noteRead);
  }
  
  if(currentReadNote != 0){
    lastReadNote = currentReadNote;
  }
  currentReadNote = 0; // Reset the current note so we inorder to read 
  //Serial.println(lastReadNote);

  // Check if the last read note matches any of the tones in the received chord
  if (receivedChord.tone1 == lastReadNote 
      || receivedChord.tone2 == lastReadNote 
      || receivedChord.tone3 == lastReadNote) 
    {
      // Set dataToSend to '1' so master knows if the note read earlier was the correct note 
      dataToSend = '1';
    } else {
      dataToSend = '0';
    }
  //Serial.println(dataToSend);
}

/* 
    Function to handle the data received from master
*/
void receiveEvent(int numBytes) {
  if (numBytes == sizeof(Chord)) { // Check if the received data matches the size of the Chord structure
    char* dataReceived = (char*)malloc(sizeof(Chord)); // Allocate memory for received data
    Wire.readBytes(dataReceived, sizeof(Chord)); // Read the data into the allocated memory
    memcpy(&receivedChord, dataReceived, sizeof(Chord)); // Convert char array to Chord struct
    free(dataReceived); // Free the allocated memory of dataReceived

    // Print the received chord information to the serial monitor
    /* Serial.println("Received Chord -->");
    Serial.println("Tone 1: ");
    Serial.println(receivedChord.tone1);
    Serial.println("Tone 2: ");
    Serial.println(receivedChord.tone2);
    Serial.println("Tone 3: ");
    Serial.println(receivedChord.tone3); */
    lastReadNote = 0; // Reset the last read note
  }
}

// Function to handle the data that should be sent to the master upon request
void requestEvent() {
  Wire.write(dataToSend); // Send the data stored in dataToSend
}

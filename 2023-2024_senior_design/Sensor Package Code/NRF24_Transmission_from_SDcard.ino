#include <Wire.h>
#include <SD.h>
#include "SdFat.h"
#include <SPI.h>
#include <RF24.h>
#include <printf.h>
#include <iostream>
#include <fstream>
// #include <sstream>
#include <string>
// #include <streambuf>

using namespace std;

#define CE_PIN 9
#define CSN_PIN 10

// instantiate an object for the nRF24L01 transceiver
RF24 radio(CE_PIN, CSN_PIN);

// Let these addresses be used for the pair
uint8_t address[][6] = { "1Node", "2Node" };
// It is very helpful to think of an address as a path instead of as
// an identifying device destination

// to use different addresses on a pair of radios, we need a variable to
// uniquely identify which address this radio will use to transmit
bool radioNumber = 1;  // 0 uses address[0] to transmit, 1 uses address[1] to transmit

// Used to control whether this node is sending or receiving
bool role = true;  // true = TX role, false = RX role

// For this example, we'll be using a payload containing
// a single float number that will be incremented
// on every successful transmission
float payload = 0.0;
#define SIZE 32
char buffer[SIZE + 1];
uint8_t counter = 0;
const int chipSelect = 1;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    // some boards need to wait to ensure access to serial over USB
  }

  // initialize the transceiver on the SPI bus
  if (!radio.begin()) {
    Serial.println(F("radio hardware is not responding!!"));
    while (1) {}  // hold in infinite loop
  }

  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    while (1) {
      // No SD card, so don't do anything more - stay stuck here
    }
  }
  Serial.println("card initialized.");

  // print example's introductory prompt
  Serial.println(F("RF24/examples/GettingStarted"));

  // To set the radioNumber via the Serial monitor on startup
  Serial.println(F("Which radio is this? Enter '0' or '1'. Defaults to '0'"));
  while (!Serial.available()) {
    // wait for user input
  }
  char input = Serial.parseInt();
  radioNumber = input == 1;
  Serial.print(F("radioNumber = "));
  Serial.println((int)radioNumber);

  // role variable is hardcoded to RX behavior, inform the user of this
  Serial.println(F("*** PRESS 'T' to begin transmitting to the other node"));

  // Set the PA Level low to try preventing power supply related problems
  // because these examples are likely run with nodes in close proximity to
  // each other.
  radio.setPALevel(RF24_PA_LOW);  // RF24_PA_MAX is default.

  // save on transmission time by setting the radio to only transmit the
  // number of bytes we need to transmit a float
  radio.setPayloadSize(SIZE);  // float datatype occupies 4 bytes

  // set the TX address of the RX node into the TX pipe
  radio.openWritingPipe(address[radioNumber]);  // always uses pipe 0

  // set the RX address of the TX node into a RX pipe
  radio.openReadingPipe(1, address[!radioNumber]);  // using pipe 1

  // additional setup specific to the node's role
  if (role) {
    radio.stopListening();  // put radio in TX mode
  } else {
    Serial.println("listening...");
    radio.startListening();  // put radio in RX mode
  }

  // For debugging info
  // printf_begin();             // needed only once for printing details
  // radio.printDetails();       // (smaller) function that prints raw register values
  // radio.printPrettyDetails(); // (larger) function that prints human readable data

}

void loop() {
  // put your main code here, to run repeatedly:
  File dataFile = SD.open("MPU6050_data.csv", FILE_READ);
  // int i = 0;
  // char terminator = ",";
  // size_t max = i+1;
  while (dataFile.available()) {
    int n = dataFile.fgets(line, sizeof(line));
    if (n <= 0) {
      error("fgets failed");
    }
    if (line[n-1] != '\n' && n == (sizeof(line) - 1)) {
      error("line too long");
    }
    if (!parseLine(line)) {
      error("parseLine failed");
    }
    Serial.println();
  }


  
  // std::ifstream t("MPU6050_data.txt");
  // std::stringstream buffer2;
  // buffer2 << t.rdbuf();
  // while (i < 3000) {
  //     Serial.write(dataFile.readStringUntil(terminator, max));
  //     i++;
  //   }
  // std::string file_content;
  // std::getline(std::ifstream("MPU6050_data.txt"), file_content, '\0');

  // while (dataFile.available() && dataFile.peek() != 10) { // peek returns the next character without incrementing the read index
  //   Serial.print(dataFile.read());
  // }

  // char buffer2[20];
  // uint8_t i = 0;

  // while (dataFile.available() && dataFile.peek() != '\n' && i < 19)    // note how this also prevents the buffer from overflowing (18 max to leave space for '\0'!)
  // {
  //     buffer2[i] = dataFile.read();
  //     i++;
  // }
  // buffer2[i + 1] = '\0';    // it is good practice to end strings with a terminating NULL character
  // Serial.println(buffer2[20]);

  // int i = 0;
 
  // while (i < 20) {
  //   	Serial.write(dataFile.read());
  //     i++;
  //   }

  if (radio.available()) {      // is there a payload?
      radio.read(&buffer, SIZE);  // fetch payload from FIFO
      Serial.print(F("Received: "));
      Serial.print(buffer);  // print the payload's value
      Serial.print(F(" - "));
      Serial.println(counter++);  // print the received counter
    }
}

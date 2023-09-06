  /* 
   1 - GND
   2 - VCC 3.3V !!! NOT 5V
   3 - CE to Arduino pin 9
   4 - CSN to Arduino pin 10
   5 - SCK to Arduino pin 13
   6 - MOSI to Arduino pin 11
   7 - MISO to Arduino pin 12
   8 - UNUSED
 

/*-----( Import needed libraries )-----*/
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

/*-----( Declare Constants and Pin Numbers )-----*/
#define CE_PIN   9
#define CSN_PIN 10
#define MISO_PIN 11
#define MOSI_PIN 12
#define SCK_PIN 13

// NOTE: the "LL" at the end of the constant is "LongLong" type

const uint64_t pipe_RX = 0xE8E8F0F0E1LL; // Define the transmit pipe NOTE: the "LL" at the end of the constant is "LongLong" type
const uint64_t pipe_TX = 0xE7E7E7E7E7LL; // Define the receiver pipe


// declare constants to store command received

// char newCmd[32];
 const char* boardID = "P4yd1RJhDfSDUS02";              // ID of the board 
 
 // variables for switches
 int sw1_pin = 4;
 int sw2_pin = 5;
 int sw3_pin = 6; 
 int sw4_pin = 8;  
 int sw5_pin = A0; // Analog Switch
 
/*-----( Declare objects )-----*/

RF24 radio(CE_PIN, CSN_PIN); // Create a Radio

/*----.{Declare variable to store command}...*/

void setup()   /****** SETUP: RUNS ONCE ******/
{
  
  Serial.begin(9600);
  // setting switches
  Serial.println("Setting up");    
  pinMode(sw1_pin, OUTPUT);       
  pinMode(sw2_pin, OUTPUT);       
  pinMode(sw3_pin, OUTPUT);         
  pinMode(sw4_pin, OUTPUT);       

  Serial.println("Nrf24L01 Receiver Starting");
  radio.begin();
  radio.openReadingPipe(1,pipe_RX);
  radio.startListening();
  
  // making all relays default off  
    digitalWrite(sw1_pin,LOW);  
    digitalWrite(sw2_pin,LOW);  
    digitalWrite(sw3_pin,LOW);  
    digitalWrite(sw4_pin,LOW);  
    Serial.println("nRF listening..");
}//--(end setup )---


void loop()   /****** LOOP: RUNS CONSTANTLY ******/
{
//  Serial.println("in loop..");  
  if ( radio.available() )
  {
        Serial.println("radio available");
    // Read the data payload until we've received everything
    bool done = false;
    char newCmd[32];       // variable to receive cmd       
    while (!done)
    {
      done = radio.read( &newCmd, sizeof(newCmd) ); 
          Serial.println(newCmd);
    }
    const char* cmdForBoard = newCmd;     // covert the string to char array
    if(cmdForTheBoard(cmdForBoard)) {
//         mgCmd(newCmd);      
         updateSwitches(cmdForBoard);         
     }
  } 
  else
  {    
//      Serial.println("No radio available");
  }
    
}//--(end main loop )---

/*-----( Declare User-written Functions )-----*/

void mgCmd(char task[]) {
Serial.println(task);  
}

// function to check if the command is for this board
boolean cmdForTheBoard(const char* cmdForBoard) {
  
//  const char* cmdForBoard = c.c_str();     // covert the string to char array
  char filter[17];                         // store the filter part
  boolean cmdFilter=true;                  // boolean for while loop
  int cmdFilter_ = 1;                      // for while progression
  while(cmdFilter) {            
    if(cmdForBoard[cmdFilter_] == '#') {          // check if char is '#'
      cmdFilter = false;
      break;
    }
    else {
      filter[cmdFilter_] = cmdForBoard[cmdFilter_];
      cmdFilter_++;
    }
  }
  
  if(!strcmp(filter,boardID)) {
    return false;
  }
  else {
   return true;
  }
}

// function to update switches
void updateSwitches(const char* cmd_s) {
  int switchNo = cmd_s[18]-'0';
  int switchValue = cmd_s[20]-'0';
  switch(switchNo) {
    case  1:
      Serial.println("Setting Switch 1");
      digitalWrite(sw1_pin,switchValue);  
      break;
    case  2:
      Serial.println("Setting Switch 2");
      digitalWrite(sw2_pin,switchValue);        
      break;
    case  3:
      Serial.println("Setting Switch 3");
      digitalWrite(sw3_pin,switchValue);        
      break;
    case  4:
      Serial.println("Setting Switch 4");
      digitalWrite(sw4_pin,switchValue);        
      break;
    case  5:
      Serial.println("Setting Switch 5");
      digitalWrite(sw5_pin,switchValue);        
      break;     
  }

}
//NONE
//*********( THE END )***********


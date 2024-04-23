#include <SPI.h>
#include <RH_RF95.h>
//In this instructable I don't use a sensor to generate data. If you wan't to
//send sensordata you need to attach a sensor and write this to a variable
//Set the temp variable.
float variable = 5.0;

//Pin 11 on the arduino is het MOSI pin master out slave in for data from arduino to radio
//Pin 12 on the arduino is the MISO pin master in slave out for data from radio to the arduino 
//Pin 13 on the arduino is the sck pin is the clock pin

//Following pins can be any digital pin on the arduino
//Pin 2 is the reset pin for the radio low sets it into reset
#define RESET 9

//Pin 3 used for interrupt request notification
#define DIO0 2

//Pin 4 is dropped low to start SPI transaction
#define NSS 10

//Define your frequency.
//Check the allowed frequencies in your country and the 
//frequency on the back of your board.
#define FREQ 868.0 //433.0 //915.0

//construct the lora connection
RH_RF95 rf95(NSS, DIO0);

void setup()
{
  //Reset the module
  pinMode(RESET, OUTPUT);
  digitalWrite(RESET, HIGH);

  //Start serial communication
  while(!Serial);
  Serial.begin(9600);
  delay(100);

  digitalWrite(RESET, LOW);
  delay(10);
  digitalWrite(RESET, HIGH);
  delay(10);
  
  //initialize the lora connection
  while(!rf95.init()) {
  Serial.println("Initializing...");
  while (1);
  }
  Serial.println("Succeeded");

  //set the lora frequency
  if (!rf95.setFrequency(FREQ)) {
    Serial.println("Cant set frequency");
    while (1);
  }
  Serial.print("Set freq to: "); Serial.println(FREQ);
  //set the transmission power from 5 to 23
  rf95.setTxPower(5, false);
}

void loop()
{
  //send the value of the variable variable to the server
  Serial.println("sending to receiver");
  char radiopacket[20]= "";
  //convert a float value to a char
  dtostrf(variable,5,2,radiopacket);
  Serial.print("Sending "); 
  Serial.println(radiopacket);
  Serial.println("Sending..."); 
  delay(10);
  rf95.send((uint8_t *) radiopacket, 20);
  Serial.println("waiting for packet to complete....");
  delay(10);
  rf95.waitPacketSent();

  //waiting for reply
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);

  Serial.println("Waiting for reply...");
  delay(10);
  if(rf95.waitAvailableTimeout(1000))
  {
    //Print the message
    if(rf95.recv(buf, &len))
    {
      Serial.print("Got reply: ");
      Serial.println((char*)buf);
      //If you like to print the signal strength
      //Serial.println("RSSI: " rf95.lastRssi(), DEC);
    }
    else
    {
      Serial.println("failed");
    }
  }
  else
  {
    Serial.println("no reply");
  }
  delay(1000);
}

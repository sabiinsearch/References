//This code was written by Nematic !
//Youtube : Nematic !
int ledpin=5;    // This is output pin ( this code can be used for dimming leds ) 
int pot=A0;      // mid pin of potentiometer to analog pin 0 at nano   
int potvalue;

void setup()
{
  pinMode(ledpin, OUTPUT);    // pin 5 is o/p
  pinMode(pot, INPUT);        // analog pin 0 is input
}
void loop()
{
  potvalue=analogRead(pot);
  potvalue=map(potvalue,0,1023,0,255);
  analogWrite(ledpin, potvalue);
}


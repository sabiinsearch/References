
int M1_pin1 = 2;
int M1_pin2 = 4;
int M1_pwm_pin = 3;

int M2_pin1 = 9;
int M2_pin2 = A2;
int M2_pwm_pin = 5;

int M3_pin1 = 7;
int M3_pin2 = 8;
int M3_pwm_pin = 6;


void setup() {
  // put your setup code here, to run once:

  pinMode(M1_pin1,OUTPUT);
  pinMode(M1_pin2,OUTPUT);
  pinMode(M2_pin1,OUTPUT);
  pinMode(M2_pin2,OUTPUT);
  pinMode(M3_pin1,OUTPUT);
  pinMode(M3_pin2,OUTPUT);

  pinMode(M1_pwm_pin,INPUT);
  pinMode(M2_pwm_pin,INPUT);
  pinMode(M3_pwm_pin,INPUT);
  
}

void loop() {

  Serial.begin(9600);
  Serial.println("Testing Start..");
  
//  analogWrite(M1_pwm_pin,255);
//  analogWrite(M2_pwm_pin,255);
  analogWrite(M3_pwm_pin,255);
  
////  Motor 1
//   digitalWrite(M1_pin1,HIGH);
//   digitalWrite(M1_pin2,LOW);
//   
//   delay(1500);
//    
//   digitalWrite(M1_pin1,LOW);
//   digitalWrite(M1_pin2,HIGH);

//   //  Motor 2
//   digitalWrite(M2_pin1,HIGH);
//   digitalWrite(M2_pin2,LOW);
//   
//   delay(1500);
//    
//   digitalWrite(M2_pin1,LOW);
//   digitalWrite(M2_pin2,HIGH);

   //  Motor 3
   digitalWrite(M3_pin1,HIGH);
   digitalWrite(M3_pin2,LOW);
   
   delay(1500);
    
   digitalWrite(M3_pin1,LOW);
   digitalWrite(M3_pin2,HIGH);
   
Serial.println("Testing done..");
}

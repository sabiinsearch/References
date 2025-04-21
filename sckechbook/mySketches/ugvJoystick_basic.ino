/*

Driver to send vintage joystick inputs to 915 MHz telemetry link
to send to RC car

References:
http://www.built-to-spec.com/blog/2009/09/10/using-a-pc-joystick-with-the-arduino/
https://old.pinouts.ru/InputCables/GameportPC_pinout.shtml
https://baremetalmicro.com/tutorial_avr_digital_io/04-Outputs.html

*/

//#define DEBUG_MODE //debug_mode shows raw ADC 
#define STEER_ADC A0
#define THROTTLE_ADC A3
#define LED 10
#define TRIGGER 2
#define BLINK_TIME 250
#define BUTTON 7

char tx_array[24] = {};
unsigned long blink_ctr = 0;

#define MOVING_AVG_FILTER_NUM 5
int smooth_array_steer[MOVING_AVG_FILTER_NUM];
int smooth_array_throttle[MOVING_AVG_FILTER_NUM];
int index_steer = 0;
int sum_steer = 0;
int index_throttle = 0;
int sum_throttle = 0;
int smooth_steer = 0;
int smooth_throttle = 0;

void setup()
{

  Serial.begin(57600);
  pinMode(STEER_ADC,INPUT);
  pinMode(THROTTLE_ADC,INPUT);
  pinMode(LED,OUTPUT);
  pinMode(TRIGGER,INPUT);
  pinMode(BUTTON,INPUT);
}

int mapVal(int curr_val, int minIn, int maxIn, int minOut, int maxOut)
{
  return (minOut + ((maxOut - minOut) / (maxIn - minIn) * (curr_val - minIn)));
}

// ADC (steer)
int tx_steer()
{
  return mapVal(analogRead(STEER_ADC),960,670,1000,2000);
}

// moving average filter for ADC (throttle)
int tx_throttle()
{
  sum_throttle = sum_throttle - smooth_array_throttle[index_throttle];
  uint8_t temp_value = analogRead(THROTTLE_ADC);
  smooth_array_throttle[index_throttle] = temp_value;
  sum_throttle += temp_value;
  index_throttle = (index_throttle+1) % MOVING_AVG_FILTER_NUM;
  smooth_throttle = sum_throttle / MOVING_AVG_FILTER_NUM;
  int converted_throttle = mapVal(smooth_throttle,27,252,1000,1240);
  if (converted_throttle > 1200) return 1200;
  else return converted_throttle;
}

void loop()
{
  
  if (digitalRead(TRIGGER) == LOW)
  {
    if (millis() - blink_ctr > BLINK_TIME)
    {
      blink_ctr = millis();
      digitalWrite(LED,HIGH);     
    }
    else
    {
      digitalWrite(LED,LOW);
    }
     
    #ifdef DEBUG_MODE
      sprintf(tx_array,"T: %d S: %d\n",analogRead(THROTTLE_ADC),analogRead(STEER_ADC));
    #else
      sprintf(tx_array,"%d,%d#\n",tx_throttle(),tx_steer());
    #endif
  }
  else
  {
    sprintf(tx_array,"%d,%d#\n",990,1500);
  }
  Serial.print(tx_array);    
  delay(50);  
}

#define SDA_PORT PORTC
#define SDA_PIN 4
#define SCL_PORT PORTC
#define SCL_PIN 5
#define I2C_SLOWMODE 2
#include <SoftI2CMaster.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

// standard I2C address for Smart Battery packs
byte deviceAddress = 11;

Adafruit_PCD8544 display = Adafruit_PCD8544(7, 6, 5, 4, 3);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2

#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH  16

// Standard and common non-standard Smart Battery commands
#define BATTERY_MODE             0x03
#define TEMPERATURE              0x08
#define VOLTAGE                  0x09
#define CURRENT                  0x0A
#define RELATIVE_SOC             0x0D
#define ABSOLUTE_SOC             0x0E
#define REMAINING_CAPACITY       0x0F
#define FULL_CHARGE_CAPACITY     0x10
#define TIME_TO_FULL             0x13
#define CHARGING_CURRENT         0x14
#define CHARGING_VOLTAGE         0x15
#define BATTERY_STATUS           0x16
#define CYCLE_COUNT              0x17
#define DESIGN_CAPACITY          0x18
#define DESIGN_VOLTAGE           0x19
#define SPEC_INFO                0x1A
#define MFG_DATE                 0x1B
#define SERIAL_NUM               0x1C
#define MFG_NAME                 0x20   // String
#define DEV_NAME                 0x21   // String
#define CELL_CHEM                0x22   // String
#define MFG_DATA                 0x23   // String
#define MFG_DATA3B               0x4F   // String
//#define MFG_DATA3C               0x3C   // String
//#define MFG_DATA3D               0x3D   // String
//#define MFG_DATA3E               0x3E   // String
//#define MFG_DATA30               0x48   // String
//#define MFG_DATA31               0x49   // String
//#define MFG_DATA32               0x4A   // String
//#define MFG_DATA33               0x4B   // String
#define CELL8_VOLTAGE            0x43
#define CELL7_VOLTAGE            0x42
#define CELL6_VOLTAGE            0x41
#define CELL5_VOLTAGE            0x40
#define CELL4_VOLTAGE            0x3C   // Indidual cell voltages don't work on Lenovo and Dell Packs
#define CELL3_VOLTAGE            0x3D
#define CELL2_VOLTAGE            0x3E
#define CELL1_VOLTAGE            0x3F
#define STATE_OF_HEALTH          0x54
#define bufferLen 32
uint8_t i2cBuffer[bufferLen];

void setup()
{
    display.begin();
   display.setContrast(50);

  Serial.begin(9600);

   Serial.println(i2c_init());
  // rotation example
  display.clearDisplay();
  display.setRotation(2);  // rotate 90 degrees counter clockwise, can also use values of 2 and 3 to go further.
  //display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(0,0);
 // display.println("Rotation");
  display.setTextSize(1);
          display.println("   ");
  display.println("    Battery ");
    display.println("    Checker");
        display.println("   ");
        display.println("   by PBTech   ");
        display.println("     V1.0   ");
  display.display();
  delay(1000);
    display.setTextSize(1);
      display.clearDisplay();
  while (!Serial) {    
    ; // wait for Serial port to connect.
  }

  Serial.println("Serial Initialized");
  
  i2c_init();
  Serial.println("I2C Inialized");
//  scan();  //bypassed but left in for future projects

}

int fetchWord(byte func)
{
  i2c_start(deviceAddress<<1 | I2C_WRITE);
  i2c_write(func);
  i2c_rep_start(deviceAddress<<1 | I2C_READ);
  byte b1 = i2c_read(false);
  byte b2 = i2c_read(true);
  i2c_stop();
  return (int)b1|((( int)b2)<<8);
}

uint8_t i2c_smbus_read_block ( uint8_t command, uint8_t* blockBuffer, uint8_t blockBufferLen ) 
{
  uint8_t x, num_bytes;
  i2c_start((deviceAddress<<1) + I2C_WRITE);
  i2c_write(command);
  i2c_rep_start((deviceAddress<<1) + I2C_READ);
  num_bytes = i2c_read(false); // num of bytes; 1 byte will be index 0
  num_bytes = constrain(num_bytes,0,blockBufferLen-2); // room for null at the end
  for (x=0; x<num_bytes-1; x++) { // -1 because x=num_bytes-1 if x<y; last byte needs to be "nack"'d, x<y-1
    blockBuffer[x] = i2c_read(false);
  }
  blockBuffer[x++] = i2c_read(true); // this will nack the last byte and store it in x's num_bytes-1 address.
  blockBuffer[x] = 0; // and null it at last_byte+1
  i2c_stop();
  return num_bytes;
}

void scan()  //bypassed but left in for future projects
{
  byte i = 0;
  for ( i= 0; i < 127; i++  )
  {
    bool ack = i2c_start(i<<1 | I2C_WRITE); 
    if ( ack ) {
          Serial.print("Address: 0x");
    Serial.print(i,HEX);

      Serial.println(": OK");
      Serial.flush();
    }
    else {
   
    }
    i2c_stop();
  }
}

void loop()
{
  uint8_t length_read = 0;
   display.clearDisplay();
  Serial.print("Manufacturer Name: ");
  //  display.print("Manu: ");
 
  length_read = i2c_smbus_read_block(MFG_NAME, i2cBuffer, bufferLen);
  //display.println(Serial.write(i2cBuffer, length_read));
  Serial.write(i2cBuffer, length_read);
  Serial.println(" ");

  Serial.print("Device Name: ");
  length_read = i2c_smbus_read_block(DEV_NAME, i2cBuffer, bufferLen);
  Serial.write(i2cBuffer, length_read);
  Serial.println("");

  Serial.print("Chemistry ");
  length_read = i2c_smbus_read_block(CELL_CHEM, i2cBuffer, bufferLen);
  Serial.write(i2cBuffer, length_read);
  Serial.println("");

  Serial.print("Manufacturer Data ");
  length_read = i2c_smbus_read_block(MFG_DATA, i2cBuffer, bufferLen);
  Serial.write(i2cBuffer, length_read);

  Serial.println("");
  

    Serial.print("Manufacturer Data 4F");

  Serial.print("  ");
   Serial.print(fetchWord(MFG_DATA3B));
    Serial.println(" % ");


  
  Serial.print("Design Capacity: " );
  Serial.println(fetchWord(DESIGN_CAPACITY));
 
      
   Serial.print("Current Capacity: " );
  Serial.println(fetchWord(FULL_CHARGE_CAPACITY));
  Serial.print("Design Voltage: " );
  Serial.println(fetchWord(DESIGN_VOLTAGE));

  String formatted_date = "Manufacture Date (Y-M-D): ";
  int mdate = fetchWord(MFG_DATE);
  int mday = B00011111 & mdate;
  int mmonth = mdate>>5 & B00001111;
  int myear = 1980 + (mdate>>9 & B01111111);
  formatted_date += myear;
  formatted_date += "-";
  formatted_date += mmonth;
  formatted_date += "-";
  formatted_date += mday;
  Serial.println(formatted_date);

  Serial.print("Serial Number: ");
  Serial.println(fetchWord(SERIAL_NUM));

  Serial.print("Specification Info: ");
  Serial.println(fetchWord(SPEC_INFO));

    Serial.print("State of Health: ");
  Serial.println(fetchWord(STATE_OF_HEALTH));

  Serial.print("Battery Mode (BIN): 0b");
  Serial.println(fetchWord(BATTERY_MODE),BIN);

  Serial.print("Battery Status (BIN): 0b");
  Serial.println(fetchWord(BATTERY_STATUS),BIN);

   Serial.print("Max Charging Current: (mA)");
  Serial.print(fetchWord(CHARGING_CURRENT));
  Serial.print("   ");
  
  Serial.print("Max Charging Voltage::::: ");
  Serial.println(fetchWord(CHARGING_VOLTAGE));
  Serial.println("");
  Serial.println("*********Current data*********");
   Serial.print("Cycle Count: " );
  Serial.println(fetchWord(CYCLE_COUNT));

   
  Serial.print("Voltage: ");
  Serial.println((float)fetchWord(VOLTAGE)/1000);

    display.print("Yr=");
    display.print(myear);
    display.print(" Cy=");
    display.println(fetchWord(CYCLE_COUNT));
    display.print("DC=");
    display.print((float(fetchWord(DESIGN_CAPACITY))/1000),1);
    display.print("  CC=");
    display.println((float(fetchWord(FULL_CHARGE_CAPACITY))/1000),1);
    display.print("DV=");
    display.print((float(fetchWord(DESIGN_VOLTAGE))/1000),1);
    display.print(" V=");
    display.println((float(fetchWord(VOLTAGE))/1000),1);
    
  Serial.print("Remaining Capacity: " );
  Serial.println(fetchWord(REMAINING_CAPACITY));

  Serial.print("Relative Charge(%): ");
  Serial.print(fetchWord(RELATIVE_SOC));
  Serial.print("   ");
  
  Serial.print("Absolute Charge(%): ");
  Serial.println(fetchWord(ABSOLUTE_SOC));
  
  Serial.print("Minutes remaining for full charge: ");
  Serial.println(fetchWord(TIME_TO_FULL));

//Extended commands may not work with some battery packs - always show first 2 battery voltages
  Serial.print("Cell 1 Voltage: ");
  Serial.print(fetchWord(CELL1_VOLTAGE));
  display.setTextColor(WHITE, BLACK);
  display.print("1");  
  display.setTextColor(BLACK);
  display.print((fetchWord(CELL1_VOLTAGE))/10);
  Serial.print("   ");
  Serial.print("Cell 2 Voltage: ");
  Serial.print(fetchWord(CELL2_VOLTAGE));
  Serial.print("   ");
  display.setTextColor(WHITE, BLACK);
  display.print("2");  
  display.setTextColor(BLACK);
  display.print((fetchWord(CELL2_VOLTAGE))/10);
  if ((fetchWord(VOLTAGE))>10000){  // cell 3 shown if greater than 10volts
    Serial.print("Cell 3 Voltage: ");
    Serial.println(fetchWord(CELL3_VOLTAGE));
    display.setTextColor(WHITE, BLACK);
    display.print("3");
      display.setTextColor(BLACK);
    display.println((fetchWord(CELL3_VOLTAGE))/10);
    if ((fetchWord(VOLTAGE))>14000){
      Serial.print("Cell 4 Voltage: ");
      Serial.print(fetchWord(CELL4_VOLTAGE));
      display.setTextColor(WHITE, BLACK);
      display.print("4");
      display.setTextColor(BLACK);  
      display.print((fetchWord(CELL4_VOLTAGE))/10);
      Serial.print("   ");
      if ((fetchWord(VOLTAGE))>18000){
        Serial.print("Cell 5 Voltage: ");
        Serial.print(fetchWord(CELL5_VOLTAGE));
        display.setTextColor(WHITE, BLACK);
        display.print("5");  
        display.setTextColor(BLACK);
        display.print((fetchWord(CELL5_VOLTAGE))/100);
        if ((fetchWord(VOLTAGE))>22000){
          Serial.print("   ");
          Serial.print("Cell 6 Voltage: ");
          Serial.println(fetchWord(CELL6_VOLTAGE));
          display.setTextColor(WHITE, BLACK);
          display.print("6");
          display.setTextColor(BLACK);  
          display.println((fetchWord(CELL6_VOLTAGE))/10);
          if ((fetchWord(VOLTAGE))>26000){  
            Serial.print("Cell 7 Voltage: ");
            Serial.print(fetchWord(CELL7_VOLTAGE));
            Serial.print("   ");
            display.setTextColor(WHITE, BLACK);
            display.print("7");  
            display.setTextColor(BLACK);
            display.print((fetchWord(CELL7_VOLTAGE))/10);
            if ((fetchWord(VOLTAGE))>30000){
              Serial.print("Cell 8 Voltage: ");
              Serial.println(fetchWord(CELL8_VOLTAGE));
              display.setTextColor(WHITE, BLACK);
              display.print("8"); 
              display.setTextColor(BLACK); 
              display.print((fetchWord(CELL8_VOLTAGE))/10);
            }
          }
        }
      } 
    }
    display.display();
   }
  Serial.println();
  Serial.print("Temp: ");
  unsigned int tempk = fetchWord(TEMPERATURE);
  Serial.println((float)tempk/10.0-273.15);
  Serial.print("Current (mA): " );
  Serial.println(fetchWord(CURRENT));
  Serial.println("**********************");
  Serial.println("   ");
  Serial.println("   ");
  delay(5000);
}

// Based on code from Richard Baker https://github.com/rtbaker
#include <Wire.h>
// State
volatile uint16_t count;
unsigned long rpm;
unsigned long timeold;
//volatile bool count_overflow;
int interpin;

unsigned long timenow;
unsigned long diff;
const float sfr = 10; // Sampling frequency in Hz

// Setup
int sensorPin = 2; // Pin D2
#define SLAVE_ADDRESS          0x29 //slave address,any number from 0x01 to 0x7F

typedef struct reg_map_t {
  uint32_t rpm;  // Data
};

const int union_size = sizeof(reg_map_t);

// buffer
typedef union _buffer_t{
  reg_map_t map;
  uint8_t bytes[union_size];
};

static _buffer_t _buffer;

void setup()
{
  pinMode(2, INPUT_PULLUP);
  interpin = digitalPinToInterrupt(sensorPin);
  attachInterrupt(interpin, magnet_detect, FALLING); //Initialize the intterrupt pin 
  
  count = 0;
  rpm = 0;
  timeold = 0;
  //count_overflow = 0;
  
  //tone(2, 500); // Generates a test signal on the pin, 500 pulses per second
  
  // setup the I2C slave
  Wire.begin(SLAVE_ADDRESS);
  Wire.onRequest(requestEvent);
  //Wire.onReceive(receiveEvent);
  
  Serial.begin(115200);
  Serial.println("Setup complete !");
      
}

void loop() 
{
  
  //Update RPM at 10Hz (100ms) If >1000ms, change counter from byte to uint
  delay(1000.0/sfr);

  //Don't process interrupts during calculations with counter on uint (2 bytes)(I have found it doesn't seem to make a difference, but I leave just in case because it should)
  detachInterrupt(interpin);

  // Time between now and previous display update. Micros will reset at 70min!!!
  timenow = micros();
  diff = timenow - timeold;

  rpm = (60.0*1000000.0/diff)*count; 

  _buffer.map.rpm = rpm;

  //Serial.print("Counter overflow: ");
  //Serial.println(count_overflow);
  Serial.println(count);
  Serial.print("RPM = ");
  Serial.print(rpm);
  Serial.print("\n\r");

  // Reset
  count = 0;
  timeold = timenow;
  attachInterrupt(interpin, magnet_detect, RISING); //Initialize the intterrupt pin 
 
}

void magnet_detect() //This function is called whenever a magnet/interrupt is detected by the arduino
{
  count++;
  //if (count == 255) count_overflow = 1;
}

void requestEvent()
{
  
  Wire.write(_buffer.bytes, sizeof(_buffer));

}

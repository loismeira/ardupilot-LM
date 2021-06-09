// State
volatile unsigned long timenow; //Micros will reset at 70min
volatile unsigned long timeold;
volatile uint16_t diff;
volatile uint16_t diffold;
volatile unsigned long rpm;
int interpin;
int i;

// Setup
int sensorPin = 2; // Pin D2
int NM = 7; // number of magnets / pole pairs (Nr of Poles/2)
// For the EagleTree sensors, the color coding is: BLACK - VDD, RED - GND, WHITE - SIGNAL
// The T-Motor MT2814 770KV has 14 poles

void setup()
{
  pinMode(2, INPUT_PULLUP);
  interpin = digitalPinToInterrupt(sensorPin);
  attachInterrupt(interpin, magnet_detect, FALLING); //Initialize the intterrupt pin

  rpm = 0;
  timeold = 0;
  timenow = micros();
  diff = 0;

  //tone(2, 11000); // Generates a test signal on the pin, X pulses per second - Instability starts at 10kHz

  Serial.begin(115200); // Print to serial console
  Serial.println("Setup complete !");

}

void loop()
{

  //Update RPM at 10Hz (100ms) If >1000ms, change counter from byte to uint or it will overflow
  //delay(1000.0/sfr);
  delay(100); // Update at 1Hz

  //Don't process interrupts during calculations with counter on uint (2 bytes)(I have found it doesn't seem to make a difference, but I leave just in case because it should)
  detachInterrupt(interpin);

  // Time between now and previous display update.
  diffold = diff;
  diff = timenow - timeold;
  rpm = 60 * 1000000.0 / (diff*1.0 * NM);


  // If measures are the same 3 times in a row there is no rotation (can be adjusted)
  if (diff == diffold) { 
    i++;
  } else {
    i = 0;
  }
  if (i > 6) {
    rpm = 0;
  }
  
  
  Serial.print(" RPM = ");
  Serial.print(rpm);
  Serial.print("\n\r");


  attachInterrupt(interpin, magnet_detect, FALLING); //Initialize the intterrupt pin

}

void magnet_detect() //This function is called whenever a magnet/interrupt is detected by the arduino
{
  timeold = timenow;
  timenow = micros();
}

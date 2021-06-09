// State
volatile uint16_t count;
unsigned long rpm;
unsigned long timeold;
int interpin;

unsigned long timenow;
unsigned long diff;
const float sfr = 10; // Sampling frequency in Hz

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
  
  count = 0;
  rpm = 0;
  timeold = 0;
  
  //tone(2, 500); // Generates a test signal on the pin, X pulses per second
  
  Serial.begin(115200); // Print to serial console
  Serial.println("Setup complete !");
      
}

void loop() 
{
  
  //Update RPM at 10Hz (100ms) If >1000ms, change counter from byte to uint or it will overflow
  delay(1000.0/sfr);

  //Don't process interrupts during calculations with counter on uint (2 bytes)(I have found it doesn't seem to make a difference, but I leave just in case because it should)
  detachInterrupt(interpin);

  // Time between now and previous display update. Micros will reset at 70min
  timenow = micros();
  diff = timenow - timeold;

  rpm = (60.0*1000000.0/diff)*count/NM; 

  Serial.print("RPM = ");
  Serial.print(rpm);
  Serial.print("\n\r");

  // Reset
  count = 0;
  timeold = timenow;
  attachInterrupt(interpin, magnet_detect, FALLING); //Initialize the intterrupt pin 
 
}

void magnet_detect() //This function is called whenever a magnet/interrupt is detected by the arduino
{
  count++;
}

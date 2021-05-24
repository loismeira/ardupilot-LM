// Based on code from Richard Baker https://github.com/rtbaker

// State
volatile byte count;
volatile long timeold;
volatile long timenow;
volatile unsigned long diff;
//volatile bool count_overflow;
unsigned long rpm;
int interpin;
unsigned long prevdiff;



// Setup
int sensorPin = 2; // Pin D2

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
  
  Serial.begin(115200);
  Serial.println("Setup complete !");
      
}

void loop() 
{
  
  
  delay(1000);

  //Don't process interrupts during calculations with counter on uint (2 bytes)(I have found it doesn't seem to make a difference, but I leave just in case because it should)
  //detachInterrupt(interpin);

  // Time between now and previous display update. Micros will reset at 70min!!!
  //now = micros();
  //diff = timenow - timeold;

  rpm = (30.0*1000000.0/diff)*30.0;

  if (diff == prevdiff) Serial.println("RPM too low or zero");

  prevdiff = diff;

  //Serial.print("Counter overflow: ");
  //Serial.println(count_overflow);
  //Serial.println(count);
  Serial.print("RPM = ");
  Serial.print(rpm);
  Serial.print("\n\r");

  // Reset
  //count = 0;
  //timeold = now;
  //attachInterrupt(interpin, magnet_detect, RISING); //Initialize the intterrupt pin 
 
}

void magnet_detect() //This function is called whenever a magnet/interrupt is detected by the arduino
{ 
  if (count == 30) timenow = micros(), count = 0, diff = timenow-timeold, timeold = timenow;
  count++; 
}

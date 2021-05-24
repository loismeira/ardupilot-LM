/*
Arduino Hall Effect Sensor Project
by Arvind Sanjeev
Please check out  http://diyhacking.com for the tutorial of this project.
DIY Hacking
*/
bool ledstate = LOW;

void setup()
{
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  //pinMode(2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(2), magnet_detect, RISING); //Initialize the intterrupt pin (Pin D2)
    
}
void loop() //Measure RPM
{

}

void magnet_detect() //This function is called whenever a magnet/interrupt is detected by the arduino
{
  Serial.println("detect");
  digitalWrite(LED_BUILTIN, ledstate);   // Change the LED state
  ledstate = !ledstate;
}

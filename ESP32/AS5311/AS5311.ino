#include "AS5311_BD.h"

AS5311 myAS5311(5,18,19); // data, clock, chip select

const int pwmInputPin = 34;  // Replace with your chosen analog input pin
const int maxPWMCount = 4095;  // Maximum PWM count for AS5311
int currentPosition1 = 0; // Initialize the absolute position counter


const int interruptPin = 34;
volatile int counter = 0;

long value = 0;
void incrementCounter() {


  // comming from high to low value 
  if (value>2048)
    counter++;
  else
    counter--;
}


void setup()
{
  Serial.begin(115200);
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), incrementCounter, RISING);
}

int posLast = 0;
     
void loop()
{
  value = myAS5311.encoder_value();

  Serial.print("measured value: ");
  Serial.println(value+counter*4096);

  if(0){
  int pwmValue = analogRead(pwmInputPin);
  Serial.print("AS5311 RElative Position: ");
  Serial.println(pwmValue);
  
  // Calculate the change in position from the previous reading
  int positionChange = pwmValue - posLast; // Assuming the midpoint is 50% duty cycle
    Serial.print("last Position: ");
  Serial.println(posLast);
  currentPosition1 = currentPosition1 + positionChange;
  Serial.print("AS5311 Absolute Position: ");
  Serial.println(currentPosition1);

  posLast = pwmValue;

  
  }
}

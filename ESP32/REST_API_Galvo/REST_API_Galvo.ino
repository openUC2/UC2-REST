#define PIN_X 25
#define PIN_Y 26

int x_start = 0;
int x_stop = 255;
int x_step = 5;
float x_delay = 100;

int y_start = 0;
int y_stop = 255;
int y_step = 1;
float y_delay = 0;
int t_start = 0;
boolean y_active = false;
boolean x_active = false;

int x_amplitude = 0;

void oscillateY( void * parameter ) {
  Serial.println(*((int*)parameter));    
  int x_amplitude_last = x_amplitude;

  int SineValues[256];

  float ConversionFactor = (2 * PI) / 256;
  float RadAngle;
  // calculate sine values
  
  for (int MyAngle = 0; MyAngle < 256; MyAngle++) {
    RadAngle = MyAngle * ConversionFactor;
    SineValues[MyAngle] = (sin(RadAngle) * 127) + 128;
  }

  while (1) {
    for (int iy = y_start; iy < y_stop; iy = iy + y_step)
    {
      if (x_amplitude_last!=x_amplitude){
        x_amplitude_last=x_amplitude;
        dacWrite(PIN_X, x_amplitude);
      }
      y_active=true;
      dacWrite(PIN_Y, SineValues[iy]);
      y_active=false;
      delay(y_delay);
    }
  }
  vTaskDelete( NULL );
}

void setup()
{
  disableCore0WDT();
  disableCore1WDT();
  
  Serial.begin(115200);
  Serial.println("Start");

  xTaskCreate(
    oscillateY,             /* Task function. */
    "oscillateY",           /* String with name of task. */
    10000,                     /* Stack size in words. */
    (void*)&x_amplitude,      /* Parameter passed as input of the task */
    2,                         /* Priority of the task. */
    NULL);

  int t_start = millis();
}


void loop()
{
  for(int ix=x_start; ix<x_stop; ix=ix+x_step){
    Serial.println(ix);
    x_amplitude = ix; 
    delay(x_delay);
    }
}

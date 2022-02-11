#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <Stepper.h>
#include <Ps3Controller.h>
#include "A4988.h"
#include <Adafruit_NeoPixel.h>
#include "pindef.h"

#define is_wifi true
#define is_ps3 true


// galvo
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

int galvo_amplitude_y = 255;
int galvo_position_x = 0;
int galvo_delay_y = 500;
// LEDs
#define LED_PIN    18
#define LED_COUNT 1
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// Setup MOTOR
// Motor steps per revolution. Most stepper_zs are 200 steps or 1.8 degrees/step
#define MOTOR_STEPS 200
#define RPM 120

#define MS1 0
#define MS2 0
#define MS3 0
#define SLEEP 0 // optional (just delete SLEEP from everywhere if not used)

#define ENABLE 13
A4988 stepper_x(MOTOR_STEPS, PIN_DIR_X, PIN_STEP_X, SLEEP, MS1, MS2, MS3);
A4988 stepper_y(MOTOR_STEPS, PIN_DIR_Y, PIN_STEP_Y, SLEEP, MS1, MS2, MS3);
A4988 stepper_z(MOTOR_STEPS, PIN_DIR_Z, PIN_STEP_Z, SLEEP, MS1, MS2, MS3);


//#define home_wifi
#ifdef home_wifi
const char *ssid = "BenMur";
const char *password = "MurBen3128";
#else
const char* ssid = "Blynk";
const char* password = "12345678";
#endif


String hostname = "ESPLENS";

/*LASER GPIO pin*/
int LASER_PIN_PLUS = 14;// 22;
int LASER_PIN_MINUS = 0;//23;


/*Lens GPIO pins*/
int LENS_X_PIN = 21;
int LENS_Z_PIN = 22;

///* topics - DON'T FORGET TO REGISTER THEM! */
int LED_val = 0;
int LENS_X_val = 0;
int LENS_Z_val = 0;
int LENS_VIBRATE = 0;
int LASER_val = 0;
int LASER_val_2 = 0;
int LED_ARRAY_val = 0;
int LENS_X_SOFI = 0;
int LENS_Z_SOFI = 0;
int STEPS = 200;
int SPEED = 0;
int colour_led = 0;

int lensval_x = 0;
int lensval_z = 0;
int laser_diff = 0;
// global switch for vibrating the lenses
int sofi_periode = 100;  // ms
int sofi_amplitude_x = 0;   // how many steps +/- ?
int sofi_amplitude_z = 0;   // how many steps +/- ?

// default values for x/z lens' positions
int lens_x_int = 0;
int lens_z_int = 0;
int laser_int = 0;
int lens_x_offset = 0;
int lens_z_offset = 0;//1000;
boolean motor_x_running = false;
boolean motor_y_running = false;
boolean motor_z_running = false;

boolean is_sofi_x = false;
boolean is_sofi_z = false;

int glob_motor_steps[] = {0, 0, 0};
int glob_amplitude_sofi = 0;
// PWM Stuff
int pwm_resolution = 15;
int pwm_frequency = 800000;//19000; //12000
int pwm_max = (int)pow(2, pwm_resolution);
// lens x-channel
int PWM_CHANNEL_X = 0;

// lens z-channel
int PWM_CHANNEL_Z = 1;

// laser-channel
int PWM_CHANNEL_LASER = 2;

// laser-channel
int PWM_CHANNEL_LASER_2 = 3;


// env variable
float temperature;

// Web server running on port 80
WebServer server(80);

// JSON data buffer
//StaticJsonDocument<2500> jsonDocument;
char buffer[2500];
DynamicJsonDocument jsonDocument(2048);


// PS3 Controler
int offset_val = 30;
int stick_ly = 0;
int stick_lx = 0;
int stick_rx = 0;
int stick_ry = 0;

boolean is_sofi = false;
boolean is_laser_red = false;
int laserval = 1000;
int lens_x = 0;
int lens_z = 0;

boolean motor_x_on = false;
boolean motor_y_on = false;
boolean motor_z_on = false;


void onConnect() {
  Serial.println("PS3 Controller Connected.");
}


void setup() {
  disableCore0WDT();
  disableCore1WDT();

  Serial.begin(115200);
  Serial.println("Start");

  // switch of the laser directly
  pinMode(LASER_PIN_PLUS, OUTPUT);
  pinMode(LASER_PIN_MINUS, OUTPUT);
  digitalWrite(LASER_PIN_PLUS, LOW);
  digitalWrite(LASER_PIN_MINUS, LOW);

  // assign a task for the motor so that it runs when we need it
  if (is_wifi) {
    Serial.println("Start runStepperTask");
    xTaskCreatePinnedToCore(
      runStepperTask,             // Task function.
      "runStepperTask",           // String with name of task.
      10000,                   // Stack size in words.
      (void*)&glob_motor_steps,      // Parameter passed as input of the task
      1,                         // Priority of the task.
      NULL,                     // Task handle.
      0);                       // core #
  }


  // initiliaze connection
  if (is_wifi) {
    connectToWiFi();
    setup_routing();
  }

   // Enable Motors
  pinMode(ENABLE, OUTPUT);
  digitalWrite(ENABLE, LOW);


  /* setup the PWM ports and reset them to 0*/
  ledcSetup(PWM_CHANNEL_X, pwm_frequency, pwm_resolution);
  ledcAttachPin(LENS_X_PIN, PWM_CHANNEL_X);
  ledcWrite(PWM_CHANNEL_X, 0);

  ledcSetup(PWM_CHANNEL_Z, pwm_frequency, pwm_resolution);
  ledcAttachPin(LENS_Z_PIN, PWM_CHANNEL_Z);
  ledcWrite(PWM_CHANNEL_Z, 0);

  ledcSetup(PWM_CHANNEL_LASER, pwm_frequency, pwm_resolution);
  ledcAttachPin(LASER_PIN_PLUS, PWM_CHANNEL_LASER);
  ledcWrite(PWM_CHANNEL_LASER, 0);

  ledcWrite(PWM_CHANNEL_LASER_2, 0);

  // test lenses
  ledcWrite(PWM_CHANNEL_Z, 5000);
  ledcWrite(PWM_CHANNEL_X, 5000);
  delay(500);

  //Set the lenses to their offset level
  ledcWrite(PWM_CHANNEL_Z, lens_z_offset);
  ledcWrite(PWM_CHANNEL_X, lens_x_offset);

  Serial.println("Initialization finished.");


  /*
     Set target motor RPM.
  */
  Serial.println("Stepper X");
  stepper_x.begin(RPM);
  stepper_x.setMicrostep(1);  // Set microstep mode to 1:1
  stepper_x.rotate(360);     // forward revolution
  stepper_x.rotate(-360);    // reverse revolution

  Serial.println("Stepper Y");
  stepper_y.begin(RPM);
  stepper_y.setMicrostep(1);  // Set microstep mode to 1:1
  stepper_y.rotate(360);     // forward revolution
  stepper_y.rotate(-360);    // reverse revolution

  Serial.println("Stepper Z");
  stepper_z.begin(RPM);
  stepper_z.setMicrostep(1);  // Set microstep mode to 1:1
  stepper_z.rotate(360);     // forward revolution
  stepper_z.rotate(-360);    // reverse revolution


  Serial.println("Start controlGalvoTask");
  xTaskCreatePinnedToCore(controlGalvoTask, "controlGalvoTask", 10000, NULL, 1, NULL, 1);
  Serial.println("Done with setting up Tasks");

  // Connect PS3 Controller
  if (is_ps3) {
    Serial.println("Connnecting to the PS3 controller, please please the magic round button in the center..");
    Ps3.attachOnConnect(onConnect);
    Ps3.begin("01:02:03:04:05:06");
  }
  Serial.println("Ready.");


  digitalWrite(ENABLE, HIGH);
  Serial.println("Starting the programm");
}

void loop() {

  /*
    for (int ix = x_start; ix < x_stop; ix = ix + x_step) {
      Serial.printlnloo(ix);
      galvo_position_x = ix;
      delay(x_delay);
    }
  */

  if (is_wifi) {
    server.handleClient();
  }


  if (is_ps3 and Ps3.isConnected()) {

    /*
       ANALOG LEFT
    */
    if ( abs(Ps3.data.analog.stick.ly) > offset_val) {
      // move_z
      stick_ly = Ps3.data.analog.stick.ly;
      stick_ly = stick_ly - sgn(stick_ly) * offset_val;
      run_motor_z(sgn(stick_ly) * 5, stick_ly * 5);
      motor_z_on = true;
    }
    else {
      if (motor_z_on) {
        motor_z_on = false;
        stick_ly = 0;
        run_motor_z(0, 0); // switch motor off;
      }
    }


    if ( abs(Ps3.data.analog.stick.lx) > offset_val) {
      // LED Stip
      stick_lx = Ps3.data.analog.stick.lx;
      //stick_lx = stick_lx + sgn(stick_lx) * offset_val;
      if ((colour_led += sgn(stick_lx) * 5) > 0 and (colour_led += sgn(stick_lx) * 5) < 255)
        colour_led += sgn(stick_lx) * 5;
      if (colour_led < 0)
        colour_led = 0;
      strip.setPixelColor(0, strip.Color(colour_led, colour_led, colour_led));
      strip.show();
      delay(100);
    }


    /*
       ANALOG right
    */
    if ( (abs(Ps3.data.analog.stick.rx) > offset_val) and not motor_y_running) {
      // move_x
      motor_x_running = true;
      stick_rx = Ps3.data.analog.stick.rx;
      stick_rx = stick_rx - sgn(stick_rx) * offset_val;
      run_motor_y(sgn(stick_rx) * 5, stick_rx * 5);
      motor_y_on = true;
    }
    else {
      if (motor_y_on) {
        motor_y_on = false;
        stick_rx = 0;
        motor_x_running = false;
        run_motor_y(0, 0); // switch motor off;
      }
    }

    if ( (abs(Ps3.data.analog.stick.ry) > offset_val) and not motor_x_running) {
      // move_y
      stick_ry = Ps3.data.analog.stick.ry;
      motor_y_running = true;
      stick_ry = stick_ry - sgn(stick_ry) * offset_val;
      run_motor_x(sgn(stick_ry) * 5, stick_ry * 5);
      motor_x_on = true;
    }
    else {
      if (motor_x_on) {
        motor_x_on = false;
        stick_ly = 0;
        motor_y_running = false;
        run_motor_x(0, 0); // switch motor off;
      }
    }

    /*
       Keypad left
    */
    if ( Ps3.data.button.left) {
      // fine lens -
      lens_x -= 1;
      delay(100);
      run_lens_x(lens_x);
    }
    if ( Ps3.data.button.right) {
      // fine lens +
      lens_x += 1;
      delay(100);
      run_lens_x(lens_x);
    }
    if ( Ps3.data.button.down) {
      // fine focus +
      run_motor_z(10, 10);
      delay(100);
      run_motor_z(0, 0);
    }
    if ( Ps3.data.button.up) {
      // fine focus -
      run_motor_z(-10, -10);
      delay(100);
      run_motor_z(0, 0);
    }
    if ( Ps3.data.button.start) {
      // reset
      lens_z = 0;
      lens_x = 0;
      run_lens_z(lens_z);
      run_lens_x(lens_x);
      strip.setPixelColor(0, strip.Color(0, 0, 0));
      strip.show();
      is_laser_red = false;
      laserval = 0;
      run_laser(0);
    }

    int offset_val_shoulder = 5;
    if ( abs(Ps3.data.analog.button.r2) > offset_val_shoulder) {
      // lens_x++ coarse
      if ((lens_x + 1000 < pwm_max)) {
        lens_x += 1000;
        run_lens_x(lens_x);
      }
      Serial.println(lens_x);
      delay(100);
    }

    if ( abs(Ps3.data.analog.button.l2) > offset_val_shoulder) {
      // lens_x-- coarse
      if ((lens_x - 1000 > 0)) {
        lens_x -= 1000;
        run_lens_x(lens_x);
      }
      Serial.println(lens_x);
      delay(100);
    }


    if ( abs(Ps3.data.analog.button.r1) > offset_val_shoulder) {
      // lens_x + semi coarse
      if ((lens_x + 100 < pwm_max)) {
        lens_x += 100;
        run_lens_x(lens_x);
        delay(100);
      }
    }
    if ( abs(Ps3.data.analog.button.l1) > offset_val_shoulder) {
      // lens_x - semi coarse
      if ((lens_x - 100 > 0)) {
        lens_x -= 100;
        run_lens_x(lens_x);
        delay(50);
      }
    }


    if ( Ps3.data.button.circle ) {
      //if(not is_laser_red){
      Serial.println("Laser on");
      is_laser_red = true;
      laserval += 200;
      run_laser(laserval);
      delay(100);
      //}

    }

    if ( Ps3.data.button.cross ) {
      if (is_laser_red) {
        Serial.println("Laser off");
        is_laser_red = false;
        laserval = 0;
        run_laser(0);
      }

    }

    if ( Ps3.data.button.triangle) {
      if (not is_sofi) {
        Serial.println("SOFI on");
        is_sofi = true;
        glob_amplitude_sofi = 300;
      }
    }

    if ( Ps3.data.button.square ) {
      if (is_sofi) {
        is_sofi = false;
        Serial.println("SOFI off");
        glob_amplitude_sofi = 0;
      }

    }

  }

}

void controlGalvoTask( void * parameter ) {
  while (1) {
    if (true) {
      ledcWrite(PWM_CHANNEL_LASER, laserval );
      dacWrite(PIN_GALVO_Y,  galvo_amplitude_y * 1); 
      delayMicroseconds(galvo_delay_y );
      dacWrite(PIN_GALVO_Y,  0); //abs(iy*2));//SineValues[iy]);
      digitalWrite(LASER_PIN_PLUS, LOW); //switch off laser again when roundtrip is reached
      delayMicroseconds(galvo_delay_y );
    }
    else {
      for (int ix = 0; ix < 1; ix ++) {
        dacWrite(PIN_GALVO_X, ix * (127));
        delayMicroseconds(50);
        ledcWrite(PWM_CHANNEL_LASER, 1000);
        for (int iy = 0; iy < 2; iy ++) {
          dacWrite(PIN_GALVO_Y, iy * 255);
          delay(1);
        }
        ledcWrite(PWM_CHANNEL_LASER, 0);
      }
    }
  }
  vTaskDelete(NULL);
}



// the motor kinda always runs but with the numbers we provide it with
void runStepperTask( void * param) {
  uint8_t* localparameters = (uint8_t*)param;
  while (is_wifi) {
    if (abs(localparameters[0]) > 0) {
      stepper_x.move(localparameters[0]);
    }
    if (abs(localparameters[1]) > 0) {
      stepper_y.move(localparameters[1]);
    }
    if (abs(localparameters[2]) > 0) {
      stepper_z.move(localparameters[2]);
    }
  }
  vTaskDelete( NULL );
}




void connectToWiFi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.setHostname(hostname.c_str()); //define hostname
  WiFi.begin(ssid, password);

  int notConnectedCounter = 0;

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
    notConnectedCounter++;
    if (notConnectedCounter > 50) { // Reset board if not connected after 5s
      Serial.println("Resetting due to Wifi not connecting...");
      ESP.restart();
    }
  }

  Serial.print("Connected. IP: ");
  Serial.println(WiFi.localIP());
}


/*

        RESTAPI RELATED


*/

void setup_routing() {
  // GET
  //  server.on("/temperature", getTemperature);
  //server.on("/env", getEnv);

  // POST
  server.on("/led", HTTP_POST, set_led);
  server.on("/move_z", HTTP_POST, move_z);
  server.on("/move_x", HTTP_POST, move_x);
  server.on("/move_y", HTTP_POST, move_y);
  server.on("/lens_x", HTTP_POST, move_lens_x);
  server.on("/lens_z", HTTP_POST, move_lens_z);
  server.on("/laser", HTTP_POST, set_laser);
  server.on("/sofi", HTTP_POST, set_sofi);
  server.on("/galvo/position_x", HTTP_POST, set_galvo_positionx);
  server.on("/galvo/amplitude_y", HTTP_POST, set_galvo_amplitudey);
  server.on("/galvo/galvodelay_y", HTTP_POST, set_galvo_delay_y);
  server.on("/identify", identify);
  // start server
  server.begin();
}



void create_json(char *tag, float value, char *unit) {
  jsonDocument.clear();
  jsonDocument["type"] = tag;
  jsonDocument["value"] = value;
  jsonDocument["unit"] = unit;
  serializeJson(jsonDocument, buffer);
}

void add_json_object(char *tag, float value, char *unit) {
  JsonObject obj = jsonDocument.createNestedObject();
  obj["type"] = tag;
  obj["value"] = value;
  obj["unit"] = unit;
}


void identify() {
  server.send(200, "application/json", "ESP32");
}

/*


   Move Lenses


*/

void move_lens_x() {
  if (server.hasArg("plain") == false) {
    //handle error here
  }
  String body = server.arg("plain");
  Serial.println(body);
  deserializeJson(jsonDocument, body);


  // Get RGB components
  int value = jsonDocument["lens_value"];

  // apply a nonlinear look-up table
  int lensval = (value + lens_x_offset); // (int)(pow(((float)(value + lens_x_offset) / (float)pwm_max), 2) * (float)pwm_max);

  Serial.print("Lens (right) X is set to: ");
  Serial.print(lensval);
  Serial.println();

  // Respond to the client
  server.send(200, "application/json", "{}");

  // Perform action
  ledcWrite(PWM_CHANNEL_X, lensval);
}


void move_lens_z() {
  if (server.hasArg("plain") == false) {
    //handle error here
  }
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);

  // Get RGB components
  int value = jsonDocument["lens_value"];

  // apply a nonlinear look-up table
  int lensval = (value + lens_x_offset);
  //  int lensval = (int)(pow(((float)(value + lens_x_offset) / (float)pwm_max), 2) * (float)pwm_max);

  Serial.print("Lens (right) Z is set to: ");
  Serial.print(lensval);
  Serial.println();

  // Respond to the client
  server.send(200, "application/json", "{}");

  // Perform action
  ledcWrite(PWM_CHANNEL_Z, lensval);
}

void run_lens_z(int lensval) {
  ledcWrite(PWM_CHANNEL_Z, lensval);
}
void run_lens_x(int lensval) {
  ledcWrite(PWM_CHANNEL_X, lensval);
}

/*


    Set Laser

*/
void set_laser() {
  if (server.hasArg("plain") == false) {
    //handle error here
  }
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);

  // Get RGB components
  laserval = jsonDocument["value"];

  //int laserval = (int)(pow(((float)(value) / (float)pwm_max), 2) * (float)pwm_max);
  // Respond to the client
  server.send(200, "application/json", "{}");

  // PErform action
  ledcWrite(PWM_CHANNEL_LASER, laserval );
  Serial.print("Laser is set to: ");
  Serial.print(laserval);
  Serial.println();
}

void run_laser(int laserval) {
  ledcWrite(PWM_CHANNEL_LASER, laserval );
}




/*

   Move Motors

*/


void move_x() {
  server.send(200, "application/json", "{}");
  if (server.hasArg("plain") == false) {
    //handle error here
  }
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);

  // Get RGB components
  int steps = jsonDocument["steps"];
  int speed = jsonDocument["speed"];
  run_motor_x(steps, speed);
}

void run_motor_x(int steps, int speed) {
  if (steps == 0) {
    // run unsupervised
    glob_motor_steps[0] = sgn(speed) * 10;
  }
  else {
    // run only the number of steps we want
    glob_motor_steps[0] = 0;
    digitalWrite(ENABLE, LOW);
    stepper_x.begin(abs(speed));
    stepper_x.rotate(steps);
    digitalWrite(ENABLE, HIGH);
  }
}


void move_y() {
  Serial.println("move_y");
  server.send(200, "application/json", "{}");
  if (server.hasArg("plain") == false) {
    //handle error here
  }
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);

  // Get RGB components
  int steps = jsonDocument["steps"];
  int speed = jsonDocument["speed"];

  run_motor_y(steps, speed);
}

void run_motor_y(int steps, int speed) {
  // Set the speed to 5 rpm:
  stepper_y.begin(abs(speed));

  if (steps == 0) {
    // run unsupervised
    glob_motor_steps[1] = sgn(speed) * 10;
  }
  else {
    // run only the number of steps we want
    glob_motor_steps[1] = 0;
    digitalWrite(ENABLE, LOW);
    stepper_y.begin(abs(speed));
    stepper_y.rotate(steps);
    digitalWrite(ENABLE, HIGH);
  }
}


void move_z() {


  server.send(200, "application/json", "{}");
  if (server.hasArg("plain") == false) {
    //handle error here
  }
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);

  // Get RGB components
  int steps = jsonDocument["steps"];
  int speed = jsonDocument["speed"];
  run_motor_z(steps, speed);
}

void run_motor_z(int steps, int speed) {
  // Set the speed to 5 rpm:
  stepper_z.begin(abs(speed));

  if (steps == 0) {
    // run unsupervised
    glob_motor_steps[2] = sgn(speed) * 10;
  }
  else {
    // run only the number of steps we want
    glob_motor_steps[2] = 0;
    digitalWrite(ENABLE, LOW);
    stepper_z.begin(abs(speed));
    stepper_z.rotate(steps);
    digitalWrite(ENABLE, HIGH);
  }
}



static inline int8_t sgn(int val) {
  if (val < 0) return -1;
  if (val == 0) return 0;
  return 1;
}


void set_sofi() {
  if (server.hasArg("plain") == false) {
    //handle error here
  }
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);

  // Get RGB components
  int activate_sofi = jsonDocument["is_sofi"];
  is_sofi = activate_sofi;

  // Respond to the client
  server.send(200, "application/json", "{}");
}


void set_led() {
  if (server.hasArg("plain") == false) {
    //handle error here
  }
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);

  // Get RGB components
  int red = jsonDocument["red"];
  int green = jsonDocument["green"];
  int blue = jsonDocument["blue"];
  Serial.print("Red: ");
  Serial.print(red);

  // Respond to the client
  server.send(200, "application/json", "{}");
}

void set_galvo_positionx(void) {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);

  // Get RGB components
  galvo_position_x = jsonDocument["value"];
  Serial.print("galvo_position_x: ");
  Serial.print(galvo_position_x);

  // Respond to the client
  server.send(200, "application/json", "{}");
}


void set_galvo_amplitudey(void) {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);

  // Get RGB components
  galvo_amplitude_y = jsonDocument["value"];
  Serial.print("galvo_amplitude_y: ");
  Serial.print(galvo_amplitude_y);

  // Respond to the client
  server.send(200, "application/json", "{}");
}

void set_galvo_delay_y(void) {
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);

  // Get RGB components
  galvo_delay_y = jsonDocument["value"];
  Serial.print("galvo_delay_y: ");
  Serial.print(galvo_delay_y);

  // Respond to the client
  server.send(200, "application/json", "{}");
}
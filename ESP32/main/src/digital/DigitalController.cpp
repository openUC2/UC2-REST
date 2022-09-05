#include "DigitalController.h"
// Custom function accessible by the API
void DigitalController::digital_act_fct(int digitalid,int digitalval) {
  // here you can do something
  Serial.println("digital_act_fct");
  isBusy = true;
  int triggerdelay=10;

  if (DEBUG) {
    Serial.print("digitalid "); Serial.println(digitalid);
    Serial.print("digitalval "); Serial.println(digitalval);
  }

  if (digitalid == 1) {
    digital_val_1 = digitalval;
    if (digitalval == -1) {
      // perform trigger
      digitalWrite(pins->digital_PIN_1, HIGH);
      delay(triggerdelay);
      digitalWrite(pins->digital_PIN_1, LOW);
    }
    else {
      digitalWrite(pins->digital_PIN_1, digital_val_1);
      Serial.print("digital_PIN "); Serial.println(pins->digital_PIN_1);
    }
  }
  else if (digitalid == 2) {
    digital_val_2 = digitalval;
    if (digitalval == -1) {
      // perform trigger
      digitalWrite(pins->digital_PIN_2, HIGH);
      delay(triggerdelay);
      digitalWrite(pins->digital_PIN_2, LOW);
    }
    else {
      digitalWrite(pins->digital_PIN_2, digital_val_2);
      Serial.print("digital_PIN "); Serial.println(pins->digital_PIN_2);
    }
  }
  else if (digitalid == 3) {
    digital_val_3 = digitalval;
    if (digitalval == -1) {
      // perform trigger
      digitalWrite(pins->digital_PIN_3, HIGH);
      delay(triggerdelay);
      digitalWrite(pins->digital_PIN_3, LOW);
    }
    else {
      digitalWrite(pins->digital_PIN_3, digital_val_3);
      Serial.print("digital_PIN "); Serial.println(pins->digital_PIN_3);
    }
  }
}

void DigitalController::digital_set_fct(int digitalid,int digitalpin) {
  // here you can set parameters

  if (DEBUG) Serial.print("digitalid "); Serial.println(digitalid);
  if (DEBUG) Serial.print("digitalpin "); Serial.println(digitalpin);

  if (digitalid != NULL and digitalpin != NULL) {
    if (digitalid == 1) {
      pins->digital_PIN_1 = digitalpin;
      pinMode(pins->digital_PIN_1, OUTPUT);
      digitalWrite(pins->digital_PIN_1, LOW);
    }
    else if (digitalid == 2) {
      pins->digital_PIN_2 = digitalpin;
      pinMode(pins->digital_PIN_2, OUTPUT);
      digitalWrite(pins->digital_PIN_2, LOW);
    }
    else if (digitalid == 3) {
      pins->digital_PIN_3 = digitalpin;
      pinMode(pins->digital_PIN_3, OUTPUT);
      digitalWrite(pins->digital_PIN_3, LOW);
    }
  }
  isBusy = false;

}

// Custom function accessible by the API
void DigitalController::digital_get_fct(DynamicJsonDocument * jsonDocument) {
  // GET SOME PARAMETERS HERE
  int digitalid = (*jsonDocument)["digitalid"];
  int digitalpin = 0;
  int digitalval = 0;

  if (digitalid == 1) {
    if (DEBUG) Serial.println("digital 1");
    digitalpin = pins->digital_PIN_1;
    digitalval = digital_val_1;
  }
  else if (digitalid == 2) {
    if (DEBUG) Serial.println("AXIS 2");
    if (DEBUG) Serial.println("digital 2");
    digitalpin = pins->digital_PIN_2;
    digitalval = digital_val_2;
  }
  else if (digitalid == 3) {
    if (DEBUG) Serial.println("AXIS 3");
    if (DEBUG) Serial.println("digital 1");
    digitalpin = pins->digital_PIN_3;
    digitalval = digital_val_3;
  }

  jsonDocument->clear();
  (*jsonDocument)["digitalid"] = digitalid;
  (*jsonDocument)["digitalval"] = digitalval;
  (*jsonDocument)["digitalpin"] = digitalpin;
}

void DigitalController::setupDigital() {
  Serial.println("Setting Up digital");
  /* setup the output nodes and reset them to 0*/
  pinMode(pins->digital_PIN_1, OUTPUT);

  digitalWrite(pins->digital_PIN_1, HIGH);
  delay(50);
  digitalWrite(pins->digital_PIN_1, LOW);

  pinMode(pins->digital_PIN_2, OUTPUT);
  digitalWrite(pins->digital_PIN_2, HIGH);
  delay(50);
  digitalWrite(pins->digital_PIN_2, LOW);

  pinMode(pins->digital_PIN_3, OUTPUT);
  digitalWrite(pins->digital_PIN_3, HIGH);
  delay(50);
  digitalWrite(pins->digital_PIN_3, LOW);

}

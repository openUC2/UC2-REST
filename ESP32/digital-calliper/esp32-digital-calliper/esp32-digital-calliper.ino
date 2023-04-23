int i;

int sign;

long value;

float result;

int clockpin = 18;  

int datapin = 19;

unsigned long tempmicros;

 

 

void setup() {

  Serial.begin(115200);

  pinMode(clockpin, INPUT);

  pinMode(datapin, INPUT);

}



 void loop () {

  while (digitalRead(clockpin)==HIGH) {} //if clock is LOW wait until it turns to HIGH

  tempmicros=micros();

  while (digitalRead(clockpin)==LOW) {} //wait for the end of the HIGH pulse

  if ((micros()-tempmicros)>500) { //if the HIGH pulse was longer than 500 micros we are at the start of a new bit sequence

    decode(); //decode the bit sequence

  }

}

 

void decode() {

  sign=1;

  value=0;


  for (i=0;i<23;i++) {

    while (digitalRead(clockpin)==HIGH) { } //wait until clock returns to HIGH- the first bit is not needed

    while (digitalRead(clockpin)==LOW) {} //wait until clock returns to LOW

    if (digitalRead(datapin)==LOW) {

      if (i<20) {

        value|= 1<<i;

      }

      if (i==20) {

        sign=-1;

      }

    }

  }

  result=(value*sign)/100.00;    

  Serial.println(result); //print result with 2 decimals

  delay(50);

}                                                                                                                                                                               

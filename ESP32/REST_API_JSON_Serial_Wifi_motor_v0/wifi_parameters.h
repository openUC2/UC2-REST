#include "WiFi.h"

// setting up wifi parameters
const char *mSSID = "Blynk"; //"BenMur"; //
const char *mPWD = "12345678"; // "MurBen3128";//
String hostname = "ESPLENS";


void connectToWiFi() {
  Serial.print("Connecting to ");
  Serial.println(mSSID);
  WiFi.mode(WIFI_STA);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.setHostname(hostname.c_str()); //define hostname
  WiFi.begin(mSSID, mPWD);

  int notConnectedCounter = 0;

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.println("Wifi connecting...");
    notConnectedCounter++;
    if (notConnectedCounter > 50) { // Reset board if not connected after 5s
      Serial.println("Resetting due to Wifi not connecting...");
      ESP.restart();
    }
  }
  Serial.print("Connected. IP: ");
  Serial.println(WiFi.localIP());
}

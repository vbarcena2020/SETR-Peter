/*|----------------------------------------------------------|*/
/*|Connection sketch to eduroam network (WPA/WPA2) Enteprise |*/
/*|Suitable for almost any ESP32 microcontroller with WiFi   |*/
/*|Raspberry or Arduino WiFi CAN'T USE THIS LIBRARY!!!       |*/
/*|Edited by: Martin Chlebovec (martinius96)                 |*/
/*|Compilation under 2.0.3 Arduino Core and higher worked    |*/
/*|Compilation can be done only using STABLE releases        |*/
/*|Dev releases WILL NOT WORK. (Check your Ard. Core .json)  |*/
/*|WiFi.begin() have more parameters for PEAP connection     |*/
/*|----------------------------------------------------------|*/


#include <WiFi.h> //Wifi library
#include <ArduinoJson.h>
#include <ArduinoMqttClient.h>
#include "arduino_secrets/arduino_secrets.h"


// Recommendation. The following information is sensitive. Better
// if you save those variables in a different header file and make sure that
// new file is not pushed at any github public repository
#define EAP_ANONYMOUS_IDENTITY "20220719anonymous@urjc.es" // leave as it is
#define EAP_IDENTITY "m.pancracio.2020@alumnos.urjc.es"    // Use your URJC email
#define EAP_USERNAME "m.pancracio.2020@alumnos.urjc.es"    // Use your URJC email
//SSID NAME
const char* ssid = "eduroam"; // eduroam SSID


//-----
WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);


const char broker[] = "193.147.53.2";
int        port     = 21883;
const char topic[]  = "/SETR/2023/4/";


const long interval = 8000;
unsigned long previousMillis = 0;

int count = 0;

StaticJsonDocument<256> doc;

char out[128];

void send_start_lap() {
  doc["team_name"] = "Nocom-Pila";
  doc["id"] = 4;
  doc["action"] = "START_LAP";

  int b =serializeJson(doc, out);
  Serial.print("bytes = ");
  Serial.println(b,DEC);
}


//-----
void setup() {
  Serial.begin(115200);
  delay(10);
  Serial.print(F("Connecting to network: "));
  Serial.println(ssid);
  WiFi.disconnect(true); 

  WiFi.begin(ssid, WPA2_AUTH_PEAP, EAP_IDENTITY, EAP_USERNAME, PASSWORD_MARVIN); 

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println("");
  Serial.println(F("WiFi is connected!"));
  Serial.println(F("IP address set: "));
  Serial.println(WiFi.localIP()); //print LAN IP


  ////-------------


  Serial.println("You're connected to the network");
  Serial.println();

  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(broker);

  if (!mqttClient.connect(broker, port)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());

    while (1);
  }

  Serial.println("You're connected to the MQTT broker!");
  Serial.println();
  //-----------------------------
  // doc["team_name"] = "Nocom-Pila";
  // doc["id"] = 4;
  // doc["action"] = "START_LAP";

  // int b =serializeJson(doc, out);
  // Serial.print("bytes = ");
  // Serial.println(b,DEC);

}

void loop() {
  yield();

  mqttClient.poll();

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    // save the last time a message was sent
    previousMillis = currentMillis;

    Serial.print("Sending message to topic: ");
    Serial.println(topic);
    Serial.println(out);


    // send message, the Print interface can be used to set the message contents
    mqttClient.beginMessage(topic);
    mqttClient.print(out);
    mqttClient.endMessage();

    Serial.println();
  }
}

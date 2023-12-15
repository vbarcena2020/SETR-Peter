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
volatile int state = 0;
long time_init;
#define RXD2 33
#define TXD2 4
#define START 0
String sendBuff;

//-----
WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);


const char broker[] = "193.147.53.2";
int        port     = 21883;
const char topic[]  = "/SETR/2023/4/";
const long interval = 5000;
unsigned long previousMillis = 0;

int count = 0;

StaticJsonDocument<256> docStart;
StaticJsonDocument<256> docEnd;
StaticJsonDocument<256> docObs;
StaticJsonDocument<256> docLine;
StaticJsonDocument<256> docPing;
char out[128];

void set_start_lap() {
  docStart["team_name"] = "Nocom-Pila";
  docStart["id"] = 4;
  docStart["action"] = "START_LAP";

  int b =serializeJson(docStart, out);
  Serial.print("bytes = ");
  Serial.println(b,DEC);
}


void set_end_lap(String end_time ) {
  docEnd["team_name"] = "Nocom-Pila";
  docEnd["id"] = 4;
  docEnd["action"] = "END_LAP";
  docEnd["time"] = end_time;

  int b =serializeJson(docEnd, out);
}

void set_obs_json(String distance ) {
  docObs["team_name"] = "Nocom-Pila";
  docObs["id"] = 4;
  docObs["action"] = "OBSTACLE_DETECTED";
  docObs["distance"] = distance;

  int b =serializeJson(docObs , out);
}

void set_line_json() {
  docLine["team_name"] = "Nocom-Pila";
  docLine["id"] = 4;
  docLine["action"] = "LINE_LOST";

  int b =serializeJson(docLine, out);
}
void set_ping_json(String time) {
  docPing["team_name"] = "Nocom-Pila";
  docPing["id"] = 4;
  docPing["action"] = "PING";
  docPing["time"] = time ;

  int b =serializeJson(docPing, out);
}

//-----
void setup() {
  Serial.begin(9600);
  delay(10);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
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
  set_start_lap();
}

void loop() {
  //yield();

  mqttClient.poll();
  unsigned long currentMillis = millis();
  if (state == 0) {
    

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
      time_init = millis();
      delay(5000);
      //Serial2.println('a');
      
      Serial2.println("start");
      Serial.println("start");

      state = 1;
    }
  }
  if (state == 1) {

    String s = Serial2.readString();
    //String time = Serial2.readString();

    int i = s.indexOf(' ');
    int j = s.indexOf('\r');
    String token = s.substring(0, i);
    String parameter = s.substring(i+1, j);
    //Serial.println(token);
    //Serial.println(parameter);
    //Serial.println(s);
  
    if(token == "ping") {
      
      set_ping_json(parameter);
      //Serial.println(parameter);
      mqttClient.beginMessage(topic);
      mqttClient.print(out);
      mqttClient.endMessage();
    } else if(token == "lost") {
      set_line_json();
      //Serial.println(parameter);
      mqttClient.beginMessage(topic);
      mqttClient.print(out);
      mqttClient.endMessage();
      
    } else if(token == "dist") {
      
      set_obs_json(parameter);
      //Serial.println(parameter);
      mqttClient.beginMessage(topic);
      mqttClient.print(out);
      mqttClient.endMessage();
      
    }  else if (token == "stop") {
      set_end_lap(parameter);
      //Serial.println(parameter);
      mqttClient.beginMessage(topic);
      mqttClient.print(out);
      mqttClient.endMessage();
      state = 2;
      
    }
    
  }

}

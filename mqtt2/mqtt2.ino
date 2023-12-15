#include <ArduinoMqttClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>





///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = "wifieif2";        // your network SSID (name)
char pass[] = "Goox0sie_WZCGGh25680000";    // your network password (use for WPA, or use as key for WEP)

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

const char broker[] = "193.147.53.2";
int        port     = 21883;
const char topic[]  = "/SETR/2023/4/";

//set interval for sending messages (milliseconds)
const long interval = 8000;
unsigned long previousMillis = 0;

int count = 0;

StaticJsonDocument<256> doc;



char out[128];


void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // attempt to connect to Wifi network:
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    // failed, retry
    Serial.print(".");
    delay(500);
  }

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
  doc["team_name"] = "Nocom-Pila";
  doc["id"] = 4;
  doc["action"] = "START_LAP";

  int b =serializeJson(doc, out);
  Serial.print("bytes = ");
  Serial.println(b,DEC);

}

void loop() {
  // call poll() regularly to allow the library to send MQTT keep alive which
  // avoids being disconnected by the broker
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
#define PIN_ITR20001LEFT   A2
#define PIN_ITR20001MIDDLE A1
#define PIN_ITR20001RIGHT  A0



void setup() {
 
  pinMode(PIN_ITR20001RIGHT, INPUT);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  int SensorValue = digitalRead(PIN_ITR20001RIGHT);
  
  Serial.print("SensorPin Value: ");
  Serial.println(SensorValue);
  delay(1000);


}

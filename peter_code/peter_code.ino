#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include "FastLED.h"

#define PIN_RBGLED 4
#define NUM_LEDS 1

#define TRIG_PIN 13  
#define ECHO_PIN 12  
#define PIN_ITR20001_LEFT   A2
#define PIN_ITR20001_MIDDLE A1
#define PIN_ITR20001_RIGHT  A0
// Enable/Disable motor control.
//  HIGH: motor control enabled
//  LOW: motor control disabled
#define PIN_Motor_STBY 3

// Group A Motors (Right Side)
// PIN_Motor_AIN_1: Digital output. HIGH: Forward, LOW: Backward
#define PIN_Motor_AIN_1 7
// PIN_Motor_PWMA: Analog output [0-255]. It provides speed.
#define PIN_Motor_PWMA 5

// Group B Motors (Left Side)
// PIN_Motor_BIN_1: Digital output. HIGH: Forward, LOW: Backward
#define PIN_Motor_BIN_1 8
// PIN_Motor_PWMB: Analog output [0-255]. It provides speed.
#define PIN_Motor_PWMB 6

#define SPEED 50
#define TURN_SPEED 20

CRGB leds[NUM_LEDS];

volatile int ir_state = 1;
volatile int last_ir_sense = 1;
volatile int stop = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT_PULLUP); 
  pinMode(PIN_ITR20001_LEFT, INPUT);
  pinMode(PIN_ITR20001_MIDDLE, INPUT);
  pinMode(PIN_ITR20001_RIGHT, INPUT);


  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT_PULLUP); 

  pinMode(PIN_Motor_STBY, OUTPUT);

  pinMode(PIN_Motor_AIN_1, OUTPUT);
  pinMode(PIN_Motor_PWMA, OUTPUT);

  pinMode(PIN_Motor_BIN_1, OUTPUT);
  pinMode(PIN_Motor_PWMB, OUTPUT); 
  
  FastLED.addLeds<NEOPIXEL, PIN_RBGLED>(leds, NUM_LEDS);
  FastLED.setBrightness(20);


  digitalWrite(PIN_Motor_STBY, HIGH);
  xTaskCreate(sense_line, "Sense_Line", 200, NULL, 1, NULL);
  xTaskCreate(sense_distance, "Sense_Distance", 100, NULL, 1, NULL);
  xTaskCreate(movement, "Movement", 200, NULL, 1, NULL);
  vTaskStartScheduler();
}

uint32_t Color(uint8_t r, uint8_t g, uint8_t b)
{
  return (((uint32_t)r << 16) | ((uint32_t)g << 8) | b);
}



void forward() {
  digitalWrite(PIN_Motor_AIN_1, HIGH);
  analogWrite(PIN_Motor_PWMA, SPEED);
  digitalWrite(PIN_Motor_BIN_1, HIGH);
  analogWrite(PIN_Motor_PWMB, SPEED);
}

void backward() {
  digitalWrite(PIN_Motor_AIN_1, LOW);
  analogWrite(PIN_Motor_PWMA, SPEED);
  digitalWrite(PIN_Motor_BIN_1, LOW);
  analogWrite(PIN_Motor_PWMB, SPEED);
}

void turn_left() {
  digitalWrite(PIN_Motor_AIN_1, HIGH);
  analogWrite(PIN_Motor_PWMA, SPEED);
  digitalWrite(PIN_Motor_BIN_1, HIGH);
  analogWrite(PIN_Motor_PWMB, TURN_SPEED);
}

void turn_right() {
  digitalWrite(PIN_Motor_AIN_1, HIGH);
  analogWrite(PIN_Motor_PWMA, TURN_SPEED);
  digitalWrite(PIN_Motor_BIN_1, HIGH);
  analogWrite(PIN_Motor_PWMB, SPEED);
}

void movement(void *pvParameters) {
  for (;;) {

  if (stop == 0) {
    switch(ir_state) {
      case 0:
        turn_right();
        break;

      case 1:
        forward();
        break;

      case 2:
        turn_left();
        break;
      
      case -1:
        switch(last_ir_sense) {
          case 0:
            turn_right();
            break;

          case 1:
            forward();
            break;

          case 2:
            turn_left();
            break;

          default:
            break;
        }
        break;

      default:
        break;
    }
  } else {
    analogWrite(PIN_Motor_PWMA, 0);
    analogWrite(PIN_Motor_PWMB, 0);
  }
  }
}

void sense_distance(void *pvParameters) {
  for (;;) {

  long duration, dist;
  digitalWrite(TRIG_PIN, LOW);  
  delayMicroseconds(4);
  digitalWrite(TRIG_PIN, HIGH);  
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  duration = pulseIn(ECHO_PIN, HIGH);  
  dist = duration * 10 / 292 / 2;
  if (dist <= 8 && dist >= 5) {
    stop = 1;
  }
  }
}


void sense_line(void *pvParameters) {
  for (;;) {

    int sensor_rigth = analogRead(PIN_ITR20001_RIGHT);
    int sensor_left = analogRead(PIN_ITR20001_LEFT);
    int sensor_middle = analogRead(PIN_ITR20001_MIDDLE);

    if(sensor_rigth > 700 && sensor_rigth > sensor_left) {
      ir_state = 0;
      last_ir_sense = 0;
      FastLED.showColor(Color(0, 255, 0));
    } else if(sensor_middle > 700 && sensor_middle > sensor_rigth && sensor_middle > sensor_left) {
      ir_state = 1;
      last_ir_sense = 1;
      FastLED.showColor(Color(0, 255, 0));
    } else if(sensor_left > 700 && sensor_left > sensor_rigth) {
      ir_state = 2;
      last_ir_sense = 2;
      FastLED.showColor(Color(0, 255, 0));
    } else {
      ir_state = -1;
      FastLED.showColor(Color(255, 0, 0));
    }
    // Serial.print("Left: ");
    // Serial.print(sensor_left);
    // Serial.print(" | ");
    // Serial.print("Middle: ");
    // Serial.print(sensor_middle);
    // Serial.print(" | ");
    // Serial.print("Rigth: ");
    // Serial.println(sensor_rigth);  
  
  }
  
    
}


void loop() {
 

  // put your main code here, to run repeatedly:
  //forward();
  //turn_right();
  //sense_line();
  //backward();
  //turn_left();
  //digitalWrite(PIN_Motor_STBY, LOW);

}

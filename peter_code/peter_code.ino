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

#define SPEED 120
#define TURN_SPEED 60

#define PERIODIC_MOVE 90   //MOVE
#define PERIODIC_SENSE 70  //SENSE
#define PERIODIC_DIST 90   //DIST

int received = 0;

CRGB leds[NUM_LEDS];

volatile int ir_state = 1;
volatile int last_ir_sense = 1;
int stop = 1;
long dist;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT_PULLUP); 
  pinMode(PIN_ITR20001_LEFT, INPUT);
  pinMode(PIN_ITR20001_MIDDLE, INPUT);
  pinMode(PIN_ITR20001_RIGHT, INPUT);


  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT); 

  pinMode(PIN_Motor_STBY, OUTPUT);

  pinMode(PIN_Motor_AIN_1, OUTPUT);
  pinMode(PIN_Motor_PWMA, OUTPUT);

  pinMode(PIN_Motor_BIN_1, OUTPUT);
  pinMode(PIN_Motor_PWMB, OUTPUT); 
  
  FastLED.addLeds<NEOPIXEL, PIN_RBGLED>(leds, NUM_LEDS);
  FastLED.setBrightness(20);


  digitalWrite(PIN_Motor_STBY, HIGH);
  xTaskCreate(sense_line, "Sense_Line", 90, NULL, 2, NULL);
  xTaskCreate(movement, "Movement", 70, NULL, 3, NULL);
  xTaskCreate(sense_distance, "Sense_Distance", 90, NULL, 1, NULL);
  xTaskCreate(comunication_esp, "Comunication_ESP", 90, NULL, 0, NULL);

}

uint32_t Color(uint8_t r, uint8_t g, uint8_t b)
{
  return (((uint32_t)r << 16) | ((uint32_t)g << 8) | b);
}

void forward(int speed) {
  digitalWrite(PIN_Motor_AIN_1, HIGH);
  analogWrite(PIN_Motor_PWMA, speed);
  digitalWrite(PIN_Motor_BIN_1, HIGH);
  analogWrite(PIN_Motor_PWMB, speed);
}

void backward(int speed) {
  digitalWrite(PIN_Motor_AIN_1, LOW);
  analogWrite(PIN_Motor_PWMA, speed);
  digitalWrite(PIN_Motor_BIN_1, LOW);
  analogWrite(PIN_Motor_PWMB, speed);
}

void turn_left(int speed, int turn_speed) {
  digitalWrite(PIN_Motor_AIN_1, HIGH);
  analogWrite(PIN_Motor_PWMA, speed);
  digitalWrite(PIN_Motor_BIN_1, HIGH);
  analogWrite(PIN_Motor_PWMB, turn_speed);
}

void turn_right(int speed, int turn_speed) {
  digitalWrite(PIN_Motor_AIN_1, HIGH);
  analogWrite(PIN_Motor_PWMA, turn_speed);
  digitalWrite(PIN_Motor_BIN_1, HIGH);
  analogWrite(PIN_Motor_PWMB, speed);
}

void movement(void *pvParameters) {
  TickType_t xLastWakeTime, aux;



  while (1) {
    xLastWakeTime = xTaskGetTickCount();
    aux = xLastWakeTime;

    if (stop == 0) {
      switch(ir_state) {
        case 0:
          turn_right(SPEED, TURN_SPEED);
          break;

        case 1:
          forward(SPEED);
          break;

        case 2:
          turn_left(SPEED, TURN_SPEED);
          break;
        
        case -1:
          switch(last_ir_sense) {
            case 0:
              turn_right(SPEED, 5);
              break;

            case 1:
              forward(SPEED);
              break;

            case 2:
              turn_left(SPEED, 5);
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
      
    xTaskDelayUntil( &xLastWakeTime, ( PERIODIC_MOVE / portTICK_PERIOD_MS ) );

  }
}

void sense_distance(void *pvParameters) {
   TickType_t xLastWakeTime, aux;


  while (1) {
    xLastWakeTime = xTaskGetTickCount();
    aux = xLastWakeTime;
    long duration;
    digitalWrite(TRIG_PIN, LOW);  
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);  
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    
    duration = pulseIn(ECHO_PIN, HIGH);  
    dist = duration * 0.034 / 2; // Speed of sound in air is approximately 34 microseconds per centimeter
    if (dist <= 8 && dist >= 5) {
      stop = 1;
    }

      
    xTaskDelayUntil( &xLastWakeTime, ( PERIODIC_DIST / portTICK_PERIOD_MS ) );
  }
}



void comunication_esp(void *pvParameters) {
  TickType_t xLastWakeTime, aux;


  while (1) {
    xLastWakeTime = xTaskGetTickCount();
    aux = xLastWakeTime;

    if (stop == 1) {
      String s = Serial.readString();
      Serial.print(s);
      //if (Serial.available() > 0) {

        //String c = Serial.readString();
        //Serial .print(c);
        
        // if (c == "a")  {            

        //   // Set Red Green to LED
        //   stop = 0;
        //   FastLED.showColor(Color(0, 0, 255));
        //   break;
        // } 

      //}

      if (s == 'a')  {            

        // Set Red Green to LED
        stop = 0;
        FastLED.showColor(Color(0, 0, 255));
        break;
      } 

    }
     
    xTaskDelayUntil( &xLastWakeTime, ( PERIODIC_DIST / portTICK_PERIOD_MS ) );
  }
}

void sense_line(void *pvParameters) {
  TickType_t xLastWakeTime, aux;

  while (1) {
    xLastWakeTime = xTaskGetTickCount();
    aux = xLastWakeTime;

    int sensor_rigth = analogRead(PIN_ITR20001_RIGHT);
    int sensor_left = analogRead(PIN_ITR20001_LEFT);
    int sensor_middle = analogRead(PIN_ITR20001_MIDDLE);

    if(sensor_rigth > 600 && sensor_rigth > sensor_left) {
      ir_state = 0;
      last_ir_sense = 0;
      FastLED.showColor(Color(0, 255, 0));
    } else if(sensor_middle > 600 && sensor_middle > sensor_rigth && sensor_middle > sensor_left) {
      ir_state = 1;
      last_ir_sense = 1;
      FastLED.showColor(Color(0, 255, 0));
    } else if(sensor_left > 600 && sensor_left > sensor_rigth) {
      ir_state = 2;
      last_ir_sense = 2;
      FastLED.showColor(Color(0, 255, 0));
    } else {
      ir_state = -1;
      FastLED.showColor(Color(255, 0, 0));
    }
      
    xTaskDelayUntil( &xLastWakeTime, ( PERIODIC_SENSE / portTICK_PERIOD_MS ) );

    
  }
  
    
}


void loop() {

}


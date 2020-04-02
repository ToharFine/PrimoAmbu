/////////////////////////
// Includes:
#include <Servo.h>
/////////////////////////
//ports:
int digital_limit_port = 0;
int digital_encoder_port = 0;
int belt_motor_port = 0;
/////////////////////////
//constants:
/////////////////////////
Servo beltMotor;
int EncoderCounter = 0;
#define getDigitalLimit() (digitalRead(digital_limit_port))
/////////////////////////

void setup() {
  pinMode(digital_limit_port, INPUT);
}

void loop() {
  
  
 
}

#include <Servo.h>
#include <Filter.h>
#include <math.h>
#include <Timer.h>
/////////////////////////
//ports:
int digital_limit_port = 9;
int digital_encoder_port = 6;
int belt_motor_port = 3;
int button_1_port = 2;
int button_2_port = 4;
int button_3_port = 5;
int dial_port = 5;
/////////////////////////
//constants:
int SPARK_TO_LOW = 1540;
int SPARK_TO_HIGH = 2000;
float manual_speed = 80;
/////////////////////////
Servo beltMotor; //A delay of 1000 Microseconds is Full Reverse, A delay of 1000 to 1460 Microseconds is Proportional Reverse, A delay of 1460 to 1540 Microseconds is neutral ,A delay of 1540 to 2000 Microseconds is Proportional Forward, A delay of 2000 Microseconds is Full Forward

Timer t;
int currentSec = 0;
int prev;
Moving_average shaftPositionFilter(5 , 0);
int shaftPosition = 0;
Moving_average shaftLimitFilter(1, 0);
boolean shaftLimit = false;
boolean changedValueEncoder = false, changedValueLimit = false;

Moving_average button1Filter(50, 0);
Moving_average button2Filter(50, 0);
Moving_average button3Filter(10, 0);
Moving_average dialFilter(10, 0);

int num;

int movingDistance = 0;
float motorSpeed = 0;
float waitingTime = 500;

boolean isCalibrating;
boolean dir = true;
boolean isWaiting = false;
/////////////////////////

void setup() {
  beltMotor.attach(belt_motor_port);

  pinMode(digital_limit_port, INPUT);
  pinMode(digital_encoder_port, INPUT);
  pinMode(button_1_port, INPUT);
  pinMode(button_2_port, INPUT);
  pinMode(button_3_port, INPUT);
  
  Serial.begin(9600);

  isCalibrating = true;
  prev = shaftPositionFilter.filter(digitalRead(digital_encoder_port));
}

void loop() {
  if(isCalibrating){
    if(!isWaiting && getShaftLimit()){
      setMotorSpeed(0);
      delay(500);
      isWaiting = true;
      }
      if(!getShaftLimit())
        isWaiting = false; 
    if(getButton1()&&getButton2()){
      motorSpeed = 0;
    } else if(getButton1()){
        motorSpeed = manual_speed;
    } else if(getButton2()){
        motorSpeed = -manual_speed;
    }else
    {
      motorSpeed = 0;
      }
    movingDistance = shaftPosition;
    if(movingDistance >= 0)
    {dir = false;
    }
  } else{
    if(getButton1()){
      waitingTime = fmax(200, waitingTime - 100);
      delay(1);
    }
    if(getButton2()){
      waitingTime = fmin(1500, waitingTime + 100);
      setMotorSpeed(0);
      delay(1);
    }
    if(dir){
      motorSpeed = manual_speed;
    } else{
      motorSpeed = -manual_speed;
    }
    if((shaftPosition >= getDial() + movingDistance && isWaiting) || getShaftLimit() )
    {dir = false;
    setMotorSpeed(0);
    delay(waitingTime);
    isWaiting = false;
    shaftPosition = movingDistance + getDial();
    } else if(!dir && shaftPosition <= movingDistance){
      dir = true;
      setMotorSpeed(0);
      delay(waitingTime);
      isWaiting = true;
      }
  }
  if(getButton3()){
    isCalibrating = !isCalibrating;
    delay(500);
  }
  
  updateInputs();
  setMotorSpeed(motorSpeed);
}


float getDial(){
  return map(dialFilter.filter(analogRead(dial_port)), 0, 1023, 3, 18);
}

boolean getButton1(){
  return 1 == button1Filter.filter(digitalRead(button_1_port));  
}

boolean getButton2(){
  return 1 == button2Filter.filter(digitalRead(button_2_port));  
}

boolean getButton3(){
 return 1 == button3Filter.filter(digitalRead(button_3_port));  
}

boolean getShaftLimit() {

  boolean flag = digitalRead(digital_limit_port); //shaftLimitFilter.filter(digitalRead(digital_limit_port));
//  if(flag && changedValueLimit)
//    {currentSec = millis();
//    changedValueLimit = false;
//    }
//   if(millis() - currentSec < 200)
//    flag = false;
//   else 
//    changedValueLimit = true;
  return flag;
  }

int getEncoderPosition(){
  return shaftPosition;
}

void updateEncoderPosition(){
//    if(changedValue){
//    if(shaftPositionFilter.filter(digitalRead(digital_encoder_port))){
//      if(motorSpeed > 0)
//      ++shaftPosition;
//      else if(motorSpeed < 0)
//      --shaftPosition;
//      changedValue = false;}}
//    if(!digitalRead(6)){
//    changedValue = true;
//    }

  if(prev != digitalRead(digital_encoder_port)){
      if(motorSpeed > 0)
      ++shaftPosition;
      else if(motorSpeed < 0)
      --shaftPosition;
}
prev = digitalRead(digital_encoder_port);
  }

void updateInputs(){
  getButton1();
  getButton2();
  getButton3();
  getDial();
  updateEncoderPosition();
  
}

void setMotorSpeed(float speed){
  if(speed >= 0){
    beltMotor.writeMicroseconds(map(speed, 0, 100, SPARK_TO_LOW, SPARK_TO_HIGH));
  }
  else{
    beltMotor.writeMicroseconds(map(100+speed, 0, 100, 1000, 1460));
  }
}

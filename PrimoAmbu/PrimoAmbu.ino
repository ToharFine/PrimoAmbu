#include <Servo.h>
#include <Filter.h>
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
float manual_speed = 60;
/////////////////////////
Servo beltMotor; //A delay of 1000 Microseconds is Full Reverse, A delay of 1000 to 1460 Microseconds is Proportional Reverse, A delay of 1460 to 1540 Microseconds is neutral ,A delay of 1540 to 2000 Microseconds is Proportional Forward, A delay of 2000 Microseconds is Full Forward
int currentSec = 0;
int prev;
Moving_average shaftPositionFilter(5 , 0);
int shaftPosition = 0;
Moving_average shaftLimitFilter(2, 0);
boolean shaftLimit = false;
boolean changedValueEncoder = false, changedValueLimit = false;

boolean prevButton1 = false;
boolean prevButton2 = false;
boolean prevButton3 = false;

Moving_average button1Filter(50, 0);
Moving_average button2Filter(50, 0);
Moving_average button3Filter(5, 0);
Moving_average dialFilter(10, 0);

int movingDistance = 0;
float motorSpeed = 0;
float waitingTime = 500;

boolean isCalibrating;
boolean dir = false;
boolean isWaiting = false;
/////////////////////////
// before the loop:
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

  if (isCalibrating) {
    //the calibration mode
    if (!isWaiting && getShaftLimit()) {
      //stops the motor for a bit if it touches the shaftLimit Sensor, so its easy to calibrate
      setMotorSpeed(0);
      motorSpeed = 0;
      delay(500);
      isWaiting = true;
    }

    if (!getShaftLimit())
      isWaiting = false;

    if (getButton1() && getButton2()) {
      motorSpeed = 0;
    } else if (getButton1()) {
      motorSpeed = manual_speed;
    } else if (getButton2()) {
      motorSpeed = -manual_speed;
    } else
    {
      motorSpeed = 0;
    }
    movingDistance = 0;
    shaftPosition = 0;
    if (movingDistance >= 0)
    {
      //      dir = false;
    }

    if (getButton3Realese() && motorSpeed == 0) {
      isCalibrating = false;

      Serial.println( movingDistance);
      Serial.println(dir);
      delay(500);
    }
  } // end of calibration code
  // the "running mode" code
  else {
    if (getButton1Realese()) {
      waitingTime = fmax(200, waitingTime - 300);
    }
    if (getButton2Realese()) {
      waitingTime = fmin(1500, waitingTime + 300);
    }

    motorSpeed = dir ? manual_speed : -manual_speed;

    if ((shaftPosition >= getDial() + movingDistance + 3 || getShaftLimit() && isWaiting) ) {
      dir = false;
      setMotorSpeed(0);
      delay(waitingTime);
      isWaiting = false;
      shaftPosition = movingDistance + getDial();
    } else if (!dir && shaftPosition <= movingDistance) {
      dir = true;
      setMotorSpeed(0);
      delay(waitingTime);
      isWaiting = true;
    }

    if (getButton3Realese()) {
      isCalibrating = true;
      setMotorSpeed(0);
      delay(500);
    }
  }
  // end of running mode code

  updateInputs();
  setMotorSpeed(motorSpeed);
}


boolean getShaftLimit() {
  boolean flag = shaftLimitFilter.filter(digitalRead(digital_limit_port));
  //  Serial.println(flag);
  return flag;
}


void setMotorSpeed(float speed) {
  if (speed >= 0) {
    beltMotor.writeMicroseconds(map(speed, 0, 100, SPARK_TO_LOW, SPARK_TO_HIGH));
  }
  else {
    beltMotor.writeMicroseconds(map(100 + speed, 0, 100, 1000, 1460));
  }
}

float getDial() {
  return map(dialFilter.filter(analogRead(dial_port)), 0, 1023, 3, 18);
}


boolean getButton1() {
  return 1 == button1Filter.filter(digitalRead(button_1_port));
}

boolean getButton1Realese() {
  boolean b1 = getButton1();
  if (!b1 && prevButton1)
  {
    prevButton1 = b1;
    return true;
  } else {
    prevButton1 = b1;
    return false;
  }
}

boolean getButton2() {
  return 1 == button2Filter.filter(digitalRead(button_2_port));
}

boolean getButton2Realese() {
  boolean b2 = getButton2();
  if (!b2 && prevButton2)
  {
    prevButton2 = b2;
    return true;
  } else {
    prevButton2 = b2;
    return false;
  }
}

boolean getButton3() {
  return 1 == button3Filter.filter(digitalRead(button_3_port));
}

boolean getButton3Realese() {
  boolean b3 = getButton3();
  if (!b3 && prevButton3)
  {
    prevButton3 = b3;
    return true;
  } else {
    prevButton3 = b3;
    return false;
  }
}

int getEncoderPosition() {
  return shaftPosition;
}

void updateEncoderPosition() {
  if (prev != digitalRead(digital_encoder_port)) {
    if (motorSpeed > 0)
      ++shaftPosition;
    else if (motorSpeed < 0)
      --shaftPosition;
  }
  prev = digitalRead(digital_encoder_port);
}

void updateInputs() {
  getButton1();
  getButton1Realese();
  getButton2();
  getButton2Realese();
  getButton3();
  getButton3Realese();
  getDial();
  updateEncoderPosition();

}

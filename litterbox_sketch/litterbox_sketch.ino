
#include "HX711.h"

#define calibration_factor -57280.0 //This value is obtained using the SparkFun_HX711_Calibration sketch

#define LOADCELL_DOUT_PIN  19
#define LOADCELL_SCK_PIN  18
HX711 scale;

#define MOTOR_STOP_WEIGHT_LBS (1.0)

const int scoopButtonPin = 13;  // the number of the pushbutton pin

const int motor1Pin = 14;
const int motor2Pin = 15;

const int hall1Pin = A2;
const int hall2Pin = A3;
const int HALL_SENSOR_LOW_THRESHOLD = 200;

enum OverallState{
  STARTUP,
  IDLE,
  SCOOPING
};

OverallState state = STARTUP;

enum ScoopingState{
  BACKWARD_MOVE,
  //TODO: add pause at top
  FORWARD_MOVE,
  //TODO: ADD move past forward?
};

enum MotorState{
  OFF,
  BACKWARD,
  FORWARD
};

ScoopingState scoopingSubstate;

int scoopButtonState;            // the current reading from the input pin
int lastScoopButtonState = LOW;  // the previous reading from the input pin

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

void RunMotor(MotorState motor)
{
  switch(motor)
  {
    case OFF:
      digitalWrite(motor1Pin, LOW);
      digitalWrite(motor2Pin, LOW);    
      break;

    case BACKWARD:
      digitalWrite(motor1Pin, HIGH);
      digitalWrite(motor2Pin, LOW);
      break;

    case FORWARD:
      digitalWrite(motor1Pin, LOW);
      digitalWrite(motor2Pin, HIGH);
      break;
  }
}

void setup() {
  pinMode(scoopButtonPin, INPUT_PULLUP);

  pinMode(motor1Pin, OUTPUT);
  pinMode(motor2Pin, OUTPUT);
  digitalWrite(motor1Pin, LOW);
  digitalWrite(motor2Pin, LOW);

  
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(calibration_factor); //This value is obtained by using the SparkFun_HX711_Calibration sketch
  scale.tare();	//Assuming there is no weight on the scale at start up, reset the scale to 0


   Serial.begin(9600); // open the serial port at 9600 bps:
  Serial.print("Nick's Litterbox"); //You can change this to kg but you'll need to refactor the calibration_factor
    Serial.println();
}


void loop() {  
  delay(10);
  
  //Sensor inputs
  bool scoopButtonPressed = false;
  {
    //Button debouncing
    int reading = digitalRead(scoopButtonPin);
    if (reading != lastScoopButtonState) {
      lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > debounceDelay) {
      if (reading != scoopButtonState) {
        scoopButtonState = reading;
        if (scoopButtonState == LOW) {
          scoopButtonPressed = true;
          Serial.print("Scoop Button\n");
        }
      }
    }
    lastScoopButtonState = reading;
  }

  bool forwardStopTriggered = false, backwardStopTriggered = false;
  {
    int hall1 = analogRead(hall1Pin);
    int hall2 = analogRead(hall2Pin);

    
    //Serial.print(hall1, DEC);
    //Serial.print("\t");
    //Serial.print(hall2, DEC);
    //Serial.print("\n");

    forwardStopTriggered = hall2 < HALL_SENSOR_LOW_THRESHOLD;
    backwardStopTriggered = hall1 < HALL_SENSOR_LOW_THRESHOLD;
  }


  bool catInBox = false;
  {
    float scaleReading = scale.get_units();
    Serial.print(scaleReading, 5); //scale.get_units() returns a float
    Serial.print(" lbs"); //You can change this to kg but you'll need to refactor the calibration_factor
    Serial.println();
    if(scaleReading > MOTOR_STOP_WEIGHT_LBS)
    {
      catInBox = true;
    }
  }

  //State transitions
  /*if(buttonPressed)
  {
    state = (state + 1) % 3;
  }*/

  switch(state)
  {
    case STARTUP:
      if(forwardStopTriggered)
      {
        state = IDLE;
      }
      break;
    
    case IDLE:
        if(scoopButtonPressed)
        {
          state = SCOOPING;
        }
      break;

    case SCOOPING:
      switch(scoopingSubstate)
      {
        case BACKWARD_MOVE:
          if(backwardStopTriggered)
          {
            scoopingSubstate = FORWARD_MOVE;
          }
          break;
        case FORWARD_MOVE:
          if(forwardStopTriggered)
          {  
            state = IDLE;          
          }
      }
      break;
  }


  //Outputs based on state
  if(catInBox)
  {
    RunMotor(OFF);
  }
  else
  {
    switch(state)
    {
      case STARTUP:
        RunMotor(FORWARD);
        break;

      case IDLE:
        RunMotor(OFF);
        break;

      case SCOOPING:
        switch(scoopingSubstate)
        {
          case BACKWARD_MOVE:
            RunMotor(BACKWARD);
            break;
          case FORWARD_MOVE:
            RunMotor(FORWARD);
            break;
        }
        break;
    }
  }

    
  /*
  if(state == 0)
  {
    digitalWrite(motor1Pin, LOW);
    digitalWrite(motor2Pin, LOW);
  }
  else if(state == 1)
  {
    digitalWrite(motor1Pin, HIGH);
    digitalWrite(motor2Pin, LOW);
  }
  else if(state == 2)
  {
    digitalWrite(motor1Pin, LOW);
    digitalWrite(motor2Pin, HIGH);
  }
  */
}

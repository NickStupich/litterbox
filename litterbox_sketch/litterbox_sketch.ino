
const int buttonPin = 5;  // the number of the pushbutton pin

const int motor1Pin = 3;
const int motor2Pin = 2;

const int hall1Pin = A5;
const int hall2Pin = A4;
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

int buttonState;            // the current reading from the input pin
int lastButtonState = LOW;  // the previous reading from the input pin

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
      digitalWrite(motor1Pin, LOW);
      digitalWrite(motor2Pin, HIGH);
      break;

    case FORWARD:
      digitalWrite(motor1Pin, HIGH);
      digitalWrite(motor2Pin, LOW);
      break;
  }
}

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);

  pinMode(motor1Pin, OUTPUT);
  pinMode(motor2Pin, OUTPUT);
  digitalWrite(motor1Pin, LOW);
  digitalWrite(motor2Pin, LOW);

   Serial.begin(9600); // open the serial port at 9600 bps:

}


void loop() {  
  delay(10);
  
  //Sensor inputs
  bool buttonPressed = false;
  {
    //Button debouncing
    int reading = digitalRead(buttonPin);
    if (reading != lastButtonState) {
      lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > debounceDelay) {
      if (reading != buttonState) {
        buttonState = reading;
        if (buttonState == LOW) {
          buttonPressed = true;
          Serial.print("Button\n");
        }
      }
    }
    lastButtonState = reading;
  }

  bool forwardStopTriggered = false, backwardStopTriggered = false;
  {
    int hall1 = analogRead(hall1Pin);
    int hall2 = analogRead(hall2Pin);

    
    Serial.print(hall1, DEC);
    Serial.print("\t");
    Serial.print(hall2, DEC);
    Serial.print("\n");

    forwardStopTriggered = hall2 < HALL_SENSOR_LOW_THRESHOLD;
    backwardStopTriggered = hall1 < HALL_SENSOR_LOW_THRESHOLD;
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
        if(buttonPressed)
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

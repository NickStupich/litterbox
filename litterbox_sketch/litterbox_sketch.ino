
#include "HX711.h"

#define calibration_factor -57280.0 //This value is obtained using the SparkFun_HX711_Calibration sketch

#define LOADCELL_DOUT_PIN  19
#define LOADCELL_SCK_PIN  18
HX711 scale;

#define MOTOR_STOP_WEIGHT_LBS (1.0)

const int scoopButtonPin = 13;  // the number of the pushbutton pin
const int flattenButtonPin = 12;

const int motor1Pin = 14;
const int motor2Pin = 15;

const int hall1Pin = A2;
const int hall2Pin = A3;
const int HALL_SENSOR_LOW_THRESHOLD = 200;

const int led0Pin = 11;
const int led1Pin = 10;
const int led2Pin = 9;
const int led3Pin = 8;

enum OverallState{
  STARTUP,
  IDLE,
  SCOOPING
};

OverallState state = STARTUP;
int scoopCounter = 0;

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
int flattenButtonState;
int lastScoopButtonState = LOW;  // the previous reading from the input pin
int lastFlattenButtonState = LOW;

int resetButtonState;
int lastResetButtonState = LOW;
unsigned long lastResetDebounceTime = 0;
unsigned long lastScoopDebounceTime = 0;
unsigned long lastFlattenDebounceTime = 0;

unsigned long resetHoldTime = 1000;
unsigned long debounceDelay = 20;

struct InputsState{
  bool scoopButtonPressed;
  bool flattenButtonPressed;
  bool catInBox;
  bool forwardStopTriggered;
  bool backwardStopTriggered;
};

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

void OutputLEDCounters(int scoopCounter)
{
  if(scoopCounter >= 0x10)
  {
    digitalWrite(led0Pin, HIGH);
    digitalWrite(led1Pin, HIGH);
    digitalWrite(led2Pin, HIGH);
    digitalWrite(led3Pin, HIGH);
  }
  else
  {
    digitalWrite(led0Pin, scoopCounter & 0x01);
    digitalWrite(led1Pin, scoopCounter & 0x02);
    digitalWrite(led2Pin, scoopCounter & 0x04);
    digitalWrite(led3Pin, scoopCounter & 0x08);
  }
}

void setup() {
  pinMode(scoopButtonPin, INPUT_PULLUP);
  pinMode(flattenButtonPin, INPUT_PULLUP);

  pinMode(motor1Pin, OUTPUT);
  pinMode(motor2Pin, OUTPUT);
  digitalWrite(motor1Pin, LOW);
  digitalWrite(motor2Pin, LOW);


  pinMode(led0Pin, OUTPUT);
  pinMode(led1Pin, OUTPUT);
  pinMode(led2Pin, OUTPUT);
  pinMode(led3Pin, OUTPUT);
  OutputLEDCounters(0);
  
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(calibration_factor); //This value is obtained by using the SparkFun_HX711_Calibration sketch
  scale.tare();	//Assuming there is no weight on the scale at start up, reset the scale to 0


  Serial.begin(9600); // open the serial port at 9600 bps:
  Serial.print("Nick's Litterbox"); 
  Serial.println();
}

InputsState GetInputsState()
{
  
  //Sensor inputs
  bool scoopButtonPressed = false;
  //Button debouncing
  int scoopReading = digitalRead(scoopButtonPin);
  if (scoopReading != lastScoopButtonState) {
    lastScoopDebounceTime = millis();
  }

  if ((millis() - lastScoopDebounceTime) > debounceDelay) {
    if (scoopReading != scoopButtonState) {
      scoopButtonState = scoopReading;
      if (scoopButtonState == LOW) {
        scoopButtonPressed = true;
        Serial.print("Scoop Button\n");
      }
    }
  }
  lastScoopButtonState = scoopReading;
  

  bool flattenButtonPressed = false;  
  //Button debouncing
  int flattenReading = digitalRead(flattenButtonPin);
  if (flattenReading != lastFlattenButtonState) {
    lastFlattenDebounceTime = millis();
  }

  if ((millis() - lastFlattenDebounceTime) > debounceDelay) {
    if (flattenReading != flattenButtonState) {
      flattenButtonState = flattenReading;
      if (flattenButtonState == LOW) {
        flattenButtonPressed = true;
        Serial.print("Flatten Button\n");
      }
    }
  }
  lastFlattenButtonState = flattenReading;


  //reset is both buttons, held for a 1 second 'debounce'
  bool resetButtonPressed = false;
  int resetReading = scoopReading & flattenReading;
  if (resetReading != lastResetButtonState) {
    lastResetDebounceTime = millis();
  }

  if ((millis() - lastResetDebounceTime) > resetHoldTime) {
    if (resetReading != resetButtonState) {
      resetButtonState = resetReading;
      if (resetButtonState == LOW) {
        resetButtonPressed = true;
        Serial.print("Reset Button\n");
      }
    }
  }
  lastResetButtonState = resetReading;
  


  int hall1 = analogRead(hall1Pin);
  int hall2 = analogRead(hall2Pin);
  bool forwardStopTriggered = hall2 < HALL_SENSOR_LOW_THRESHOLD;
  bool backwardStopTriggered = hall1 < HALL_SENSOR_LOW_THRESHOLD;
  


  bool catInBox = false;
  {
    float scaleReading = scale.get_units();
    /*
    Serial.print(scaleReading, 5); //scale.get_units() returns a float
    Serial.print(" lbs"); //You can change this to kg but you'll need to refactor the calibration_factor
    Serial.println();
    */
    if(scaleReading > MOTOR_STOP_WEIGHT_LBS)
    {
      catInBox = true;
    }
  }

  InputsState result = {scoopButtonPressed, flattenButtonPressed, catInBox, forwardStopTriggered, backwardStopTriggered};

  return result;
}

void loop() {  
  delay(10);

  InputsState is = GetInputsState();
  

  //State transitions
  if(is.flattenButtonPressed)
  {
    scoopCounter++;
  }

  switch(state)
  {
    case STARTUP:
      if(is.forwardStopTriggered)
      {
        state = IDLE;
      }
      break;
    
    case IDLE:
        if(is.scoopButtonPressed)
        {
          state = SCOOPING;
        }
      break;

    case SCOOPING:
      switch(scoopingSubstate)
      {
        case BACKWARD_MOVE:
          if(is.backwardStopTriggered)
          {
            scoopingSubstate = FORWARD_MOVE;
          }
          break;
        case FORWARD_MOVE:
          if(is.forwardStopTriggered)
          {  
            state = IDLE;          
          }
      }
      break;
  }


  //Outputs based on state
  if(is.catInBox)
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
  OutputLEDCounters(scoopCounter);
    
}

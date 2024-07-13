
#include "HX711.h"

#define calibration_factor -57280.0 //This value is obtained using the SparkFun_HX711_Calibration sketch
#define LOADCELL_DOUT_PIN  19
#define LOADCELL_SCK_PIN  18
HX711 scale;
#define MOTOR_STOP_WEIGHT_LBS (1.0)

const int hall1Pin = A2;
const int hall2Pin = A3;
const int HALL_SENSOR_LOW_THRESHOLD = 200;


const int scoopButtonPin = 13;  // the number of the pushbutton pin
const int flattenButtonPin = 12;



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

void SetupInputPins()
{
  pinMode(scoopButtonPin, INPUT_PULLUP);
  pinMode(flattenButtonPin, INPUT_PULLUP);
  
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(calibration_factor); //This value is obtained by using the SparkFun_HX711_Calibration sketch
  scale.tare();	//Assuming there is no weight on the scale at start up, reset the scale to 0
}

InputsState GetInputsState() {
  
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

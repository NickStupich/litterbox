
enum OverallState{
  STARTUP,
  IDLE,
  SCOOPING
};

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

struct InputsState{
  bool scoopButtonPressed;
  bool flattenButtonPressed;
  bool catInBox;
  bool forwardStopTriggered;
  bool backwardStopTriggered;
};

struct LitterboxState{
  OverallState state = STARTUP;
  ScoopingState scoopingSubstate;
  int scoopCounter = 0;
};

LitterboxState lbState;


void setup() {
  SetupInputPins();
  SetupOutputPins();

  
  Serial.begin(9600); // open the serial port at 9600 bps:
  Serial.print("Nick's Litterbox"); 
  Serial.println();
}

void loop() {  
  delay(10);

  //Read sensors
  InputsState is = GetInputsState();

  lbState = UpdateState(lbState, is);

  SetOutputs(lbState, is);

    
}

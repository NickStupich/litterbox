
enum OverallState{
  STARTUP,
  IDLE,
  SCOOPING,
  FLATTENING,
};

enum FlatteningState{
  FLATTEN_FORWARD_MOVE,
  FLATTEN_PAUSE,
  FLATTEN_BACKWARD_MOVE,
};

enum ScoopingState{
  SCOOP_WAIT_FOR_CAT,
  SCOOP_BACKWARD_MOVE,
  SCOOP_PAUSE,
  SCOOP_FORWARD_MOVE,
};

enum MotorState{
  OFF,
  BACKWARD,
  FORWARD
};

struct InputsState{
  bool scoopButtonPressed;
  bool flattenButtonPressed;
  bool resetButtonPressed;
  bool motorStopFlag;
  bool catInBoxFlag;
  bool forwardStopTriggered;
  bool backwardStopTriggered;
  float scaleReading;
};

struct LitterboxState{
  OverallState state = STARTUP;
  
  ScoopingState scoopingSubstate;
  FlatteningState flatteningSubstate;

  unsigned long scoopingTimerStartTime;
  unsigned long flatteningTimerStartTime;
  
  int scoopCounter = 0;
  unsigned long catEnteredBoxTime;
  bool catInBoxFlag;  
};

LitterboxState lbState;

unsigned long lastLoopTime;

void setup() {
  SetupInputPins();
  SetupOutputPins();

  
  Serial.begin(9600); // open the serial port at 9600 bps:
  Serial.print("Nick's Litterbox"); 
  Serial.println();

  lastLoopTime = millis();
}

void loop() {  
  
  unsigned long loopTime = millis();
  unsigned long elapsed_ms = loopTime - lastLoopTime;
  if(elapsed_ms > 1000) elapsed_ms = 10; //unsigned long rollover
  lastLoopTime = loopTime;

  //Read sensors
  InputsState is = GetInputsState();

  lbState = UpdateState(lbState, is, elapsed_ms);

  SetOutputs(lbState, is);
    
}

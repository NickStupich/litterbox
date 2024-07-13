
const int motor1Pin = 14;
const int motor2Pin = 15;

const int led0Pin = 11;
const int led1Pin = 10;
const int led2Pin = 9;
const int led3Pin = 8;

void SetupOutputPins()
{
  pinMode(motor1Pin, OUTPUT);
  pinMode(motor2Pin, OUTPUT);
  digitalWrite(motor1Pin, LOW);
  digitalWrite(motor2Pin, LOW);

  pinMode(led0Pin, OUTPUT);
  pinMode(led1Pin, OUTPUT);
  pinMode(led2Pin, OUTPUT);
  pinMode(led3Pin, OUTPUT);

  OutputLEDCounters(0);
}

void RunMotor(MotorState motor) {
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

void OutputLEDCounters(int scoopCounter) {
  //Serial.print(scoopCounter);
  //Serial.println();

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

//Outputs based on state
void SetOutputs(LitterboxState lbState, InputsState is)
{  
  if(is.catInBox)
  {
    RunMotor(OFF);
  }
  else
  {
    switch(lbState.state)
    {
      case STARTUP:
        RunMotor(FORWARD);
        break;

      case IDLE:
        RunMotor(OFF);
        break;

      case SCOOPING:
        switch(lbState.scoopingSubstate)
        {
          case SCOOP_BACKWARD_MOVE:
            RunMotor(BACKWARD);
            break;
          case SCOOP_FORWARD_MOVE:
            RunMotor(FORWARD);
            break;
          case SCOOP_PAUSE:
            RunMotor(OFF);
            break;
        }
        break;
      case FLATTENING:
        switch(lbState.flatteningSubstate)
        {
          case FLATTEN_FORWARD_MOVE:
            RunMotor(FORWARD);
            break;
          case FLATTEN_PAUSE:
            RunMotor(OFF);
            break;
          case FLATTEN_BACKWARD_MOVE:
            RunMotor(BACKWARD);
            break;
        }
    }
  }

  OutputLEDCounters(lbState.scoopCounter);
}
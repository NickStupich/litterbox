const int scoopingTimeAtLimitMs = 1000;
const int flattenTimeAtLimitMs = 1000;
const int flattenForwardMotorTimeMs = 3000;


//State transition type logic
LitterboxState UpdateState(LitterboxState lbState, InputsState is, unsigned long elapsed_ms)
{
  switch(lbState.state)
  {
    case STARTUP:
      if(is.forwardStopTriggered)
      {
        lbState.state = IDLE;
      }
      break;
    
    case IDLE:
      if(is.scoopButtonPressed)
      {
        lbState.state = SCOOPING;
        lbState.scoopingSubstate = SCOOP_BACKWARD_MOVE;
      }
      else if(is.flattenButtonPressed)
      {
        lbState.state = FLATTENING;
        lbState.flatteningSubstate = FLATTEN_FORWARD_MOVE;
        lbState.flatteningTimerStartTime = millis();
      }
      break;

    case SCOOPING:
      switch(lbState.scoopingSubstate)
      {
        case SCOOP_BACKWARD_MOVE:
          if(is.backwardStopTriggered)
          {
            lbState.scoopingSubstate = SCOOP_PAUSE;
            lbState.scoopingTimerStartTime = millis();
          }
          break;
        case SCOOP_PAUSE:
          if(millis() - lbState.scoopingTimerStartTime > scoopingTimeAtLimitMs)
          {
            lbState.scoopingSubstate = SCOOP_FORWARD_MOVE;
          }
          break;

        case SCOOP_FORWARD_MOVE:
          if(is.forwardStopTriggered)
          {  
            lbState.state = FLATTENING;          
            lbState.flatteningSubstate = FLATTEN_FORWARD_MOVE;
            lbState.flatteningTimerStartTime = millis();
            
            lbState.scoopCounter++;
          }
          break;

        default:
          break;
          //Serial.print("Unexpected ");
          //Serial.print(__line__);
          //Serial.println();
      }
      break;
    
    case FLATTENING:
      switch(lbState.flatteningSubstate)
      {
        case FLATTEN_FORWARD_MOVE:
          if(millis() - lbState.flatteningTimerStartTime > flattenForwardMotorTimeMs)
          {
            lbState.flatteningSubstate = FLATTEN_PAUSE;
            lbState.flatteningTimerStartTime = millis();
          }
          break;
        case FLATTEN_PAUSE:
          if(millis() - lbState.flatteningTimerStartTime > flattenTimeAtLimitMs)
          {
            lbState.flatteningSubstate = FLATTEN_BACKWARD_MOVE;
            lbState.flatteningTimerStartTime = millis();
          }
          break;
        case FLATTEN_BACKWARD_MOVE:
          if(is.forwardStopTriggered)
          {  
            lbState.state = IDLE;
          }
          break;
        default:
          //Serial.print("Unexpected ");
          //Serial.print(__line__);
          //Serial.println();
          break;
      }

  }

  return lbState;
}
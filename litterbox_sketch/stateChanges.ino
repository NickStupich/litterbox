const int scoopingTimeAtLimitMs = 1000;
const int flattenTimeAtLimitMs = 1000;
const int flattenForwardMotorTimeMs = 3000;
const long scoopingWaitForCatTimeMs = (300000);
const long minimumCatInBoxTimeMs = (10000);

const float emptyLitterboxMaxWeight = (-1.0); // lbs

//State transition type logic
LitterboxState UpdateState(LitterboxState lbState, InputsState is, unsigned long elapsed_ms)
{
  if(is.resetButtonPressed)
  {
    lbState.state = IDLE;
    lbState.scoopCounter = 0;
    scale.tare();
  }
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
      else if(!lbState.catInBoxFlag && is.catInBoxFlag)
      {
        lbState.catEnteredBoxTime = millis();
        Serial.println("entered");
        //cat entered
      }
      else if(lbState.catInBoxFlag && is.catInBoxFlag)
      {
        //pooping in progress
      }
      else if(lbState.catInBoxFlag && !is.catInBoxFlag)
      {
        if((millis() - lbState.catEnteredBoxTime) > minimumCatInBoxTimeMs)
        {
          Serial.println("POOP COMPLETED");

          lbState.state = SCOOPING;
          lbState.scoopingSubstate = SCOOP_WAIT_FOR_CAT;
          lbState.scoopingTimerStartTime = millis();
        }
        else
        {
          Serial.print("TOO SHORT: ");
          Serial.println(millis() - lbState.catEnteredBoxTime);
        }
        //cat left
      }
      break;

    case SCOOPING:
      switch(lbState.scoopingSubstate)
      {
        case SCOOP_WAIT_FOR_CAT:
          if((millis() - lbState.scoopingTimerStartTime) > scoopingWaitForCatTimeMs)
          {
            Serial.print("Estimated poop quantity: ");
            Serial.print(is.scaleReading);
            Serial.println(" lbs");

            if(is.scaleReading < emptyLitterboxMaxWeight)
            {
              Serial.println("Empty tray detected");
              lbState.state = IDLE;
            }
            else
            {
              lbState.scoopingSubstate = SCOOP_BACKWARD_MOVE;
            }
          }
          else
          {
            //Serial.println(millis() - lbState.scoopingTimerStartTime);
          }
          break;
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
            scale.tare();
          }
          break;
        default:
          //Serial.print("Unexpected ");
          //Serial.print(__line__);
          //Serial.println();
          break;
      }

  }

  lbState.catInBoxFlag = is.catInBoxFlag;

  return lbState;
}
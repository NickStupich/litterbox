LitterboxState UpdateState(LitterboxState lbState, InputsState is)
{
  //State transition type logic
  if(is.flattenButtonPressed)
  {
    lbState.scoopCounter++;
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
        }
      break;

    case SCOOPING:
      switch(lbState.scoopingSubstate)
      {
        case BACKWARD_MOVE:
          if(is.backwardStopTriggered)
          {
            lbState.scoopingSubstate = FORWARD_MOVE;
          }
          break;
        case FORWARD_MOVE:
          if(is.forwardStopTriggered)
          {  
            lbState.state = IDLE;          
          }
      }
      break;
  }

  return lbState;
}
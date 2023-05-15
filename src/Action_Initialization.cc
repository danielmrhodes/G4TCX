#include "Action_Initialization.hh"
#include "Primary_Generator.hh"
#include "Run_Action.hh"
#include "Event_Action.hh"
#include "Tracking_Action.hh"

void Action_Initialization::Build() const {

  //Thread-local user classes
  SetUserAction(new Primary_Generator());
  SetUserAction(new Run_Action());
  SetUserAction(new Event_Action());
  SetUserAction(new Tracking_Action());
  
  return;
}

void Action_Initialization::BuildForMaster() const {

  //Master thread user class
  SetUserAction(new Run_Action());
  
  return;
}

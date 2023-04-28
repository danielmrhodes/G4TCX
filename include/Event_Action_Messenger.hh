#ifndef Event_Action_Messenger_h
#define Event_Action_Messenger_h 1

#include "Event_Action.hh"
#include "G4UImessenger.hh"
#include "G4UIcmdWithoutParameter.hh"
#include "G4UIcmdWithAString.hh"

class Event_Action;
class Event_Action_Messenger : public G4UImessenger {

public:
  
  Event_Action_Messenger(Event_Action* eAct);
  ~Event_Action_Messenger();
  
private:

  Event_Action* eventAction;

  void SetNewValue(G4UIcommand* command, G4String newValue);

  //Output directory
  G4UIdirectory* output_dir;
  
  G4UIcmdWithAString* name_cmd;
  G4UIcmdWithoutParameter* owc_cmd;
  
};

#endif

#ifndef Run_Action_Messenger_h
#define Run_Action_Messenger_h 1

#include "Run_Action.hh"
#include "G4UImessenger.hh"
#include "G4UIcmdWithoutParameter.hh"
#include "G4UIcmdWithAString.hh"

class Run_Action;
class Run_Action_Messenger : public G4UImessenger {

public:
  
  Run_Action_Messenger(Run_Action* runAct);
  ~Run_Action_Messenger();
  
private:

  Run_Action* runAct;

  void SetNewValue(G4UIcommand* command, G4String newValue);

  //Output directory
  G4UIdirectory* output_dir;
  
  G4UIcmdWithAString* fname_cmd;
  G4UIcmdWithAString* dname_cmd;  
  
  G4UIcmdWithoutParameter* owc_cmd;
  G4UIcmdWithoutParameter* wd_cmd;
  
};

#endif

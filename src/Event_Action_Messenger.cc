#include "Event_Action_Messenger.hh"

Event_Action_Messenger::Event_Action_Messenger(Event_Action* eAct) : eventAction(eAct) {

  //All info about the output accessed through this directory
  output_dir = new G4UIdirectory("/Output/");

  name_cmd = new G4UIcmdWithAString("/Output/FileName",this);
  name_cmd->AvailableForStates(G4ApplicationState::G4State_Idle);
  name_cmd->SetGuidance("Set output file name.");

  owc_cmd = new G4UIcmdWithoutParameter("/Output/OnlyWriteCoincidences",this);
  owc_cmd->AvailableForStates(G4ApplicationState::G4State_Idle);
  owc_cmd->SetGuidance("Only write coincidence data to file");
  
}

Event_Action_Messenger::~Event_Action_Messenger() {

  delete output_dir;
  delete name_cmd;
  delete owc_cmd;
  
}

void Event_Action_Messenger::SetNewValue(G4UIcommand* command, G4String newValue) {

  if(command == name_cmd) {
    eventAction->SetOutputFileName(newValue);
    G4cout << "Setting output file name to " << newValue << G4endl;
  }
  else if(command == owc_cmd) {
    eventAction->OWC();
    G4cout << "Only coincidence data will be written to output file." << G4endl;
  }
  
  return;
}


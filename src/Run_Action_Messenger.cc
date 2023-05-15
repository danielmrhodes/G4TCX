#include "Run_Action_Messenger.hh"

Run_Action_Messenger::Run_Action_Messenger(Run_Action* rA) : runAct(rA) {

  //All info about the output accessed through this directory
  output_dir = new G4UIdirectory("/Output/");

  name_cmd = new G4UIcmdWithAString("/Output/FileName",this);
  name_cmd->AvailableForStates(G4ApplicationState::G4State_PreInit,G4ApplicationState::G4State_Idle);
  name_cmd->SetGuidance("Set output file name.");

  owc_cmd = new G4UIcmdWithoutParameter("/Output/OnlyWriteCoincidences",this);
  owc_cmd->AvailableForStates(G4ApplicationState::G4State_PreInit,G4ApplicationState::G4State_Idle);
  owc_cmd->SetGuidance("Only write coincidence data to file");
  
}

Run_Action_Messenger::~Run_Action_Messenger() {

  delete output_dir;
  delete name_cmd;
  delete owc_cmd;
  
}

void Run_Action_Messenger::SetNewValue(G4UIcommand* command, G4String newValue) {

  G4String message = "";
  
  if(command == name_cmd) {
    runAct->SetOutputFileName(newValue);
    message = "Setting output file name to " + newValue;
  }
  else if(command == owc_cmd) {
    runAct->OWC();
    message = "Only coincidence data will be written to output file.";
  }

  G4int threadID = G4Threading::G4GetThreadId();
  if(!threadID && message != "")
    std::cout << message << std::endl;
  
  return;
}


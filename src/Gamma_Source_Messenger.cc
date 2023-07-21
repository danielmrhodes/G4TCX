#include "Gamma_Source_Messenger.hh"

Gamma_Source_Messenger::Gamma_Source_Messenger(Gamma_Source* src) : source(src) {

  source_dir = new G4UIdirectory("/Source/");

  fn_cmd = new G4UIcmdWithAString("/Source/LevelScheme",this);
  fn_cmd->AvailableForStates(G4ApplicationState::G4State_PreInit,G4ApplicationState::G4State_Idle);
  fn_cmd->SetGuidance("Set the name of the level scheme file for the gamma-ray source");

  gs_cmd = new G4UIcmdWithADouble("/Source/GroundStateSpin",this);
  gs_cmd->AvailableForStates(G4ApplicationState::G4State_PreInit,G4ApplicationState::G4State_Idle);
  gs_cmd->SetGuidance("Set the ground state spin of the gamma-ray source");

}

Gamma_Source_Messenger::~Gamma_Source_Messenger() {

  delete source_dir;
  delete fn_cmd;
  delete gs_cmd;

}

void Gamma_Source_Messenger::SetNewValue(G4UIcommand* command, G4String newValue) {

  G4String message = "";

  if(command == fn_cmd) {
    source->SetFileName(newValue);
    message = "Setting source level scheme file to " + newValue;
  }
  else if(command == gs_cmd) {
    source->SetGroundStateSpin(gs_cmd->GetNewDoubleValue(newValue));
    message = "Setting the spin of the gamma-ray source ground state to " + newValue;
  }

  G4int threadID = G4Threading::G4GetThreadId();
  if(!threadID && message != "")
    std::cout << message << std::endl;

  return;

}

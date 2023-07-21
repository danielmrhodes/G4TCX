#include "Run_Action_Messenger.hh"

Run_Action_Messenger::Run_Action_Messenger(Run_Action* rA) : runAct(rA) {

  //All info about the output accessed through this directory
  output_dir = new G4UIdirectory("/Output/");

  fname_cmd = new G4UIcmdWithAString("/Output/FileName",this);
  fname_cmd->AvailableForStates(G4ApplicationState::G4State_PreInit,G4ApplicationState::G4State_Idle);
  fname_cmd->SetGuidance("Set output file name");

  dname_cmd = new G4UIcmdWithAString("/Output/DiagnosticsFileName",this);
  dname_cmd->AvailableForStates(G4ApplicationState::G4State_PreInit,G4ApplicationState::G4State_Idle);
  dname_cmd->SetGuidance("Set name of diagnostics file");

  owc_cmd = new G4UIcmdWithoutParameter("/Output/OnlyWriteCoincidences",this);
  owc_cmd->AvailableForStates(G4ApplicationState::G4State_PreInit,G4ApplicationState::G4State_Idle);
  owc_cmd->SetGuidance("Only write coincidence data to file");

  wd_cmd = new G4UIcmdWithoutParameter("/Output/WriteDiagnostics",this);
  wd_cmd->AvailableForStates(G4ApplicationState::G4State_PreInit,G4ApplicationState::G4State_Idle);
  wd_cmd->SetGuidance("Write the diagnostic information to file");
  
}

Run_Action_Messenger::~Run_Action_Messenger() {

  delete output_dir;
  delete fname_cmd;
  delete dname_cmd;
  delete owc_cmd;
  delete wd_cmd;
}

void Run_Action_Messenger::SetNewValue(G4UIcommand* command, G4String newValue) {

  G4String message = "";
  
  if(command == fname_cmd) {
    runAct->SetOutputFileName(newValue);
    message = "Setting output file name to " + newValue;
  }
  if(command == dname_cmd) {
    runAct->SetDiagnosticsFileName(newValue);
    message = "Setting diagnostics file name to " + newValue;
  }
  else if(command == owc_cmd) {
    runAct->OnlyWriteCoincidences();
    message = "Only coincidence data will be written to output file.";
  }
  else if(command == wd_cmd) {
    runAct->WriteDiagnostics();
    message = "Diagnostic information will be written to file.";
  }

  G4int threadID = G4Threading::G4GetThreadId();
  if(!threadID && message != "")
    std::cout << message << std::endl;
  
  return;
}


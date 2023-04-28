#include "Excitation_Messenger.hh"

Excitation_Messenger::Excitation_Messenger(Excitation* exc, G4bool prj) : excitation(exc), proj(prj) {

  G4String nuc;
  G4String path;
  if(proj) {
    nuc = "projectile";
    path = "/Excitation/Projectile/";
  }
  else {
    nuc = "recoil";
    path = "/Excitation/Recoil/";
  }

  G4String cmd_name;
  G4String guidance;
  
  //All information about level schemes and excitation probabilities
  excitation_dir = new G4UIdirectory(path);

  //Level scheme file
  cmd_name = path + "LevelScheme";
  pLS_cmd = new G4UIcmdWithAString(cmd_name,this);
  pLS_cmd->AvailableForStates(G4ApplicationState::G4State_Idle);

  guidance = "Set name of the " + nuc + " level scheme file";
  pLS_cmd->SetGuidance(guidance);

  //Probabilities file
  cmd_name = path + "Probabilities";
  pPF_cmd = new G4UIcmdWithAString(cmd_name,this);
  pPF_cmd->AvailableForStates(G4ApplicationState::G4State_Idle);

  guidance ="Set name of the " + nuc + " excitation probabilities file";
  pPF_cmd->SetGuidance(guidance);
  
  //Selected state
  cmd_name = path + "PopulateState";
  pSel_cmd = new G4UIcmdWithAnInteger(cmd_name,this);
  pSel_cmd->AvailableForStates(G4ApplicationState::G4State_Idle);

  guidance = "Choose one state to always populate in the " + nuc;
  pSel_cmd->SetGuidance(guidance);

  //Considered state
  cmd_name = path + "OnlyConsiderState";
  pCon_cmd = new G4UIcmdWithAnInteger(cmd_name,this);
  pCon_cmd->AvailableForStates(G4ApplicationState::G4State_Idle);

  guidance = "Only allow one excited state to emit gamma-rays in the " + nuc
    + ". Rescale excitation probabilities";
  pCon_cmd->SetGuidance(guidance);

  //Ground state spin
  cmd_name = path + "GroundStateSpin";
  pGSS_cmd = new G4UIcmdWithADouble(cmd_name,this);
  pGSS_cmd->AvailableForStates(G4ApplicationState::G4State_Idle);

  guidance = "Set spin of the " + nuc + "  ground state";
  pGSS_cmd->SetGuidance(guidance);

  cmd_name = path + "NoFeeding";
  sCon_cmd = new G4UIcmdWithoutParameter(cmd_name,this);
  sCon_cmd->AvailableForStates(G4ApplicationState::G4State_Idle);

  guidance = "Turn off feeding to the considered state in the " + nuc
    + ". Rescale excitation probabilities";
  sCon_cmd->SetGuidance(guidance);
}

Excitation_Messenger::~Excitation_Messenger() {

  delete excitation_dir;
  
  delete pLS_cmd;
  delete pPF_cmd;
  
  delete pSel_cmd;
  delete pCon_cmd;
  delete pGSS_cmd;
  delete sCon_cmd;
  
}

void Excitation_Messenger::SetNewValue(G4UIcommand* command, G4String newValue) {

  G4String nuc;
  if(proj)
    nuc = "projectile";
  else
    nuc = "recoil";
  
  if(command == pLS_cmd) {
    excitation->SetLSFile(newValue);
    G4cout << "Setting " << nuc << " level scheme file to " << newValue << G4endl;
  }

  else if(command == pPF_cmd) {
    excitation->SetPrbFile(newValue);
    G4cout << "Setting " << nuc << " excitation probabilites file to " << newValue << G4endl;
  }

  else if(command == pSel_cmd) {
    excitation->FixState(pSel_cmd->GetNewIntValue(newValue));
    G4cout << "Selecting state " << newValue << " to populate in the " << nuc << G4endl;
  }

  else if(command == pCon_cmd) {
    excitation->OnlyConsiderState(pCon_cmd->GetNewIntValue(newValue));
    G4cout << "Will only consider state " << newValue << " when populating excited states in the "
	   << nuc << G4endl;
  }

  else if(command == pGSS_cmd) {
    excitation->SetGSS(pGSS_cmd->GetNewDoubleValue(newValue));
    G4cout << "Setting ground state spin of the " << nuc << " to " << newValue << G4endl;
  }

  else if(command == sCon_cmd) {
    excitation->SimpleConsidered();
    G4cout << "Will not include feeding to the " << nuc << " considered state" << G4endl;
  }

  return;
  
}

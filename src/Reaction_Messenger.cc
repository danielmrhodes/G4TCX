#include "Reaction_Messenger.hh"

Reaction_Messenger::Reaction_Messenger(Reaction* reac) : reaction(reac) {

  //All info about the scattering process accessed through this directory
  reaction_dir = new G4UIdirectory("/Reaction/");

  //Projectile
  beamZ_cmd = new G4UIcmdWithAnInteger("/Reaction/ProjectileZ",this);
  beamZ_cmd->AvailableForStates(G4ApplicationState::G4State_Idle);
  beamZ_cmd->SetGuidance("Set Z of projectile nucleus");

  beamA_cmd = new G4UIcmdWithAnInteger("/Reaction/ProjectileA",this);
  beamA_cmd->AvailableForStates(G4ApplicationState::G4State_Idle);
  beamA_cmd->SetGuidance("Set A of projectile nucleus");

  //Recoil
  recoilZ_cmd = new G4UIcmdWithAnInteger("/Reaction/RecoilZ",this);
  recoilZ_cmd->AvailableForStates(G4ApplicationState::G4State_Idle);
  recoilZ_cmd->SetGuidance("Set Z of recoil nucleus");

  recoilA_cmd = new G4UIcmdWithAnInteger("/Reaction/RecoilA",this);
  recoilA_cmd->AvailableForStates(G4ApplicationState::G4State_Idle);
  recoilA_cmd->SetGuidance("Set A of recoil nucleus");

  recoilThresh_cmd = new G4UIcmdWithADoubleAndUnit("/Reaction/RecoilThreshold",this);
  recoilThresh_cmd->AvailableForStates(G4ApplicationState::G4State_Idle);
  recoilThresh_cmd->SetGuidance("Set energy threshold for recoil detection.");

  //Scattering angle commands
  addTheta_cmd = new G4UIcmdWithADoubleAndUnit("/Reaction/AddThetaLAB",this);
  addTheta_cmd->AvailableForStates(G4ApplicationState::G4State_Idle);
  addTheta_cmd->SetGuidance("Add an angle to define desired LAB scattering angle range. This command must always be used two at a time, with the smaller angle coming first. Otherwise it doesn't work.");
  
}
Reaction_Messenger::~Reaction_Messenger() {

  delete reaction_dir;
  
  delete beamZ_cmd;
  delete beamA_cmd;

  delete recoilZ_cmd;
  delete recoilA_cmd;
  delete recoilThresh_cmd;
  
  delete addTheta_cmd;
  
}

void Reaction_Messenger::SetNewValue(G4UIcommand* command, G4String newValue) {

  if(command == beamZ_cmd) {
    reaction->SetBeamZ(beamZ_cmd->GetNewIntValue(newValue));
    G4cout << "Setting projectile nucleus Z to " << newValue << G4endl;
  }

  else if(command == beamA_cmd) {
    reaction->SetBeamA(beamA_cmd->GetNewIntValue(newValue));
    G4cout << "Setting projectile nucleus A to " << newValue << G4endl;
  }
  
  else if(command == recoilZ_cmd) {
    reaction->SetRecoilZ(recoilZ_cmd->GetNewIntValue(newValue));
    G4cout << "Setting recoil nucleus Z to " << newValue << G4endl;
  }
  
  else if(command == recoilA_cmd) {
    reaction->SetRecoilA(recoilA_cmd->GetNewIntValue(newValue));
    G4cout << "Setting recoil nucleus A to " << newValue << G4endl;
  }

  else if(command == recoilThresh_cmd) {
    reaction->SetRecoilThreshold(recoilThresh_cmd->GetNewDoubleValue(newValue));
    G4cout << "Setting recoil detection threshold to " << newValue << G4endl;
  }

  else if(command == addTheta_cmd) {
    reaction->AddThetaLAB(addTheta_cmd->GetNewDoubleValue(newValue));
    G4cout << "Adding theta = " << newValue << " to list of desired thetas!" << G4endl;
  }
  
  return;
}


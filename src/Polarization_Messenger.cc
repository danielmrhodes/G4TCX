#include "Polarization_Messenger.hh"

Polarization_Messenger::Polarization_Messenger(Polarization* pol, G4bool prj) : polarization(pol),
										proj(prj) {
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

  //Polarization Directory
  polar_dir = new G4UIdirectory(path);  
  
  //Statistical tensor file
  cmd_name = path + "StatisticalTensors";
  sfn_cmd = new G4UIcmdWithAString(cmd_name,this);
  sfn_cmd->AvailableForStates(G4ApplicationState::G4State_PreInit,G4ApplicationState::G4State_Idle);
  
  guidance = "Set name of " + nuc + " statistical tensor file";
  sfn_cmd->SetGuidance(guidance.c_str());

  if(proj)
    path = "/DeorientationEffect/Projectile/";
  else
    path = "/DeorientationEffect/Recoil/";
  
  //Deorentation effect paramter directory
  deo_dir = new G4UIdirectory(path);

  //Gk coefficients
  cmd_name = path + "CalculateGk";
  cgk_cmd = new G4UIcmdWithABool(cmd_name,this);
  cgk_cmd->AvailableForStates(G4ApplicationState::G4State_PreInit,G4ApplicationState::G4State_Idle);

  guidance = "Calculate Gk coefficiencts and apply them to the " + nuc + " statistical tensors";
  cgk_cmd->SetGuidance(guidance);

  //Average J
  cmd_name = path + "AverageJ";
  avj_cmd = new G4UIcmdWithADouble(cmd_name,this);
  avj_cmd->AvailableForStates(G4ApplicationState::G4State_PreInit,G4ApplicationState::G4State_Idle);

  guidance = "Set the average atomic spin of the " + nuc;
  avj_cmd->SetGuidance(guidance);

  //Gamma
  cmd_name = path + "Gamma";
  gam_cmd = new G4UIcmdWithADouble(cmd_name,this);
  gam_cmd->AvailableForStates(G4ApplicationState::G4State_PreInit,G4ApplicationState::G4State_Idle);

  guidance = "Set the FWHM of the frequency distribution (ps^-1 ) in the " + nuc;
  gam_cmd->SetGuidance(guidance);

  //Lambda
  cmd_name = path + "Lambda";
  lam_cmd = new G4UIcmdWithADouble(cmd_name,this);
  lam_cmd->AvailableForStates(G4ApplicationState::G4State_PreInit,G4ApplicationState::G4State_Idle);

  guidance = "Set the transition rate (ps^-1 ) between static and fluctuating states for the "
    + nuc;
  lam_cmd->SetGuidance(guidance);

  //TauC
  cmd_name = path + "TauC";
  tau_cmd = new G4UIcmdWithADouble(cmd_name,this);
  tau_cmd->AvailableForStates(G4ApplicationState::G4State_PreInit,G4ApplicationState::G4State_Idle);

  guidance = "Set the correlation time (ps) of the " + nuc;
  tau_cmd->SetGuidance(guidance);

  //g-factor
  cmd_name = path + "GFactor";
  gfc_cmd = new G4UIcmdWithADouble(cmd_name,this);
  gfc_cmd->AvailableForStates(G4ApplicationState::G4State_PreInit,G4ApplicationState::G4State_Idle);

  guidance = "Set the nuclear gyromagnetic ratio (g-factor) for the " + nuc;
  gfc_cmd->SetGuidance(guidance);

  //Field coefficient
  cmd_name = path + "FieldCoefficient";
  fld_cmd = new G4UIcmdWithADouble(cmd_name,this);
  fld_cmd->AvailableForStates(G4ApplicationState::G4State_PreInit,G4ApplicationState::G4State_Idle);

  guidance = "Set the hyperfine field coefficient (10^8 T) for the " + nuc;
  fld_cmd->SetGuidance(guidance);

  //Field exponent
  cmd_name = path + "FieldExponent";
  exp_cmd = new G4UIcmdWithADouble(cmd_name,this);
  exp_cmd->AvailableForStates(G4ApplicationState::G4State_PreInit,G4ApplicationState::G4State_Idle);

  guidance = "Set the hyperfine field exponent for the " + nuc;
  exp_cmd->SetGuidance(guidance);
  
}

Polarization_Messenger::~Polarization_Messenger() {

  delete polar_dir;
  delete sfn_cmd;
  
  delete cgk_cmd;
  delete deo_dir;
  delete avj_cmd;
  delete gam_cmd;
  delete lam_cmd;
  delete tau_cmd;
  delete gfc_cmd;
  delete fld_cmd;
  delete exp_cmd;
  
}

void Polarization_Messenger::SetNewValue(G4UIcommand* command, G4String newValue) {
  
  G4String nuc;
  if(proj)
    nuc = "projectile";
  else
    nuc = "recoil";

  G4String message = "";
  
  if(command == sfn_cmd) {
    polarization->SetFileName(newValue);
    message = "Setting " + nuc + " statistical tensor file to " + newValue;
  }

  ////////////Deorientation effect commands////////////
  else if(command == cgk_cmd) {
    polarization->SetCalcGk(cgk_cmd->GetNewBoolValue(newValue));
    message = "Setting flag for " + nuc + " Gk calculation to " + newValue;
  }
  
  else if(command == avj_cmd) {
    polarization->SetAverageJ(avj_cmd->GetNewDoubleValue(newValue));
    message = "Setting average atomic spin of the " + nuc + " to " + newValue;
  }

  else if(command == gam_cmd) {
    polarization->SetGamma(gam_cmd->GetNewDoubleValue(newValue));
    message = "Setting the FWHM of the frequency distribution in the " + nuc + " to " + newValue
	   + " ps^-1";
  }

  else if(command == lam_cmd) {
    polarization->SetLambdaStar(lam_cmd->GetNewDoubleValue(newValue));
    message = "Setting state fluctuation rate in the " + nuc + " to " + newValue + " ps^-1"
	  ;
  }

  else if(command == tau_cmd) {
    polarization->SetTauC(tau_cmd->GetNewDoubleValue(newValue));
    message = "Setting the correlation time in the " + nuc + " to " + newValue + " ps"
	  ;
  }

  else if(command == gfc_cmd) {
    polarization->SetGFac(gfc_cmd->GetNewDoubleValue(newValue));
    message = "Setting g-factor for the " + nuc + " to " + newValue;
  }

  else if(command == fld_cmd) {
    polarization->SetFieldCoef(fld_cmd->GetNewDoubleValue(newValue));
    message = "Setting hyperfine field coefficient in the " + nuc + " to " + newValue
	   + "*10^8 T";
  }

  else if(command == exp_cmd) {
    polarization->SetFieldExp(exp_cmd->GetNewDoubleValue(newValue));
    message = "Setting hyperfine field exponent in the " + nuc + " to " + newValue;
  }
  ////////////////////////////////////////////////////////////////

  G4int threadID = G4Threading::G4GetThreadId();
  if(!threadID && message != "")
    std::cout << message << std::endl;
  
  return;
  
}

#ifndef Polarization_Messenger_h
#define Polarization_Messenger_h 1

#include "Polarization.hh"
#include "G4UImessenger.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithADouble.hh"
#include "G4UIcmdWithABool.hh"

class Polarization;
class Polarization_Messenger : public G4UImessenger {

public:

  Polarization_Messenger(Polarization* pol, G4bool prj);
  ~Polarization_Messenger();
  
private:

  void SetNewValue(G4UIcommand* command, G4String newValue);
  
  Polarization* polarization;
  const G4bool proj;

  G4UIdirectory* polar_dir; //Polarization directory
  G4UIcmdWithAString* sfn_cmd; //Statistical tensor file name

  //Control deorientation effect parameters for projectile
  G4UIdirectory* deo_dir; //Directory
  G4UIcmdWithABool*  cgk_cmd; //Calculate Gk coefficients
  G4UIcmdWithADouble* avj_cmd; //Average J
  G4UIcmdWithADouble* gam_cmd; //Gamma
  G4UIcmdWithADouble* lam_cmd; //Lambda star
  G4UIcmdWithADouble* tau_cmd; //Tau_C
  G4UIcmdWithADouble* gfc_cmd; //g-factor
  G4UIcmdWithADouble* fld_cmd; //Field coefficient
  G4UIcmdWithADouble* exp_cmd; //Field exponent

  
};

#endif

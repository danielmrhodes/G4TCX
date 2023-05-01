#ifndef Primary_Generator_Messenger_h
#define Primary_Generator_Messenger_h 1

#include "Primary_Generator.hh"

#include "G4UImessenger.hh"
#include "G4UIcmdWithoutParameter.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithAString.hh"

class Primary_Generator;
class Primary_Generator_Messenger : public G4UImessenger {

public:

  Primary_Generator_Messenger(Primary_Generator* gen);
  ~Primary_Generator_Messenger();
  
private:

  void SetNewValue(G4UIcommand* command, G4String newValue);

  Primary_Generator* generator;

  //Incoming beam directory
  G4UIdirectory* incoming_dir;
  
  //Incoming beam commands
  G4UIcmdWithADoubleAndUnit* beamX_cmd;
  G4UIcmdWithADoubleAndUnit* beamY_cmd;
  G4UIcmdWithADoubleAndUnit* beamAX_cmd;
  G4UIcmdWithADoubleAndUnit* beamAY_cmd;
  G4UIcmdWithADoubleAndUnit* beamEn_cmd;
  
  G4UIcmdWithADoubleAndUnit* sigX_cmd;
  G4UIcmdWithADoubleAndUnit* sigY_cmd;
  G4UIcmdWithADoubleAndUnit* sigAX_cmd;
  G4UIcmdWithADoubleAndUnit* sigAY_cmd;
  G4UIcmdWithADoubleAndUnit* sigEn_cmd;

  //Scattering angle commands
  G4UIcmdWithoutParameter* toS3_cmd;
  G4UIcmdWithoutParameter* onlyR_cmd;
  G4UIcmdWithoutParameter* onlyP_cmd;

  //Inelastic scattering
  G4UIcmdWithADoubleAndUnit* inEl_cmd;

  //Mode command
  G4UIcmdWithAString* mode_cmd;

  //Update command
  G4UIcmdWithoutParameter* update_cmd;
  

};

#endif

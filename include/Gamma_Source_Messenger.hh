#ifndef Gamma_Source_Messenger_h
#define Gamma_Source_Messenger_h 1

#include "Gamma_Source.hh"
#include "G4UImessenger.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithADouble.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"

class Gamma_Source;
class Gamma_Source_Messenger : public G4UImessenger {

public:

  Gamma_Source_Messenger(Gamma_Source* src);
  ~Gamma_Source_Messenger();
  
private:

  Gamma_Source* source;
  void SetNewValue(G4UIcommand* command, G4String newValue);
  
  G4UIdirectory* source_dir; //Source directory
  
  G4UIcmdWithADoubleAndUnit* en_cmd;
  G4UIcmdWithAString* fn_cmd;
  G4UIcmdWithADouble* gs_cmd;

};

#endif

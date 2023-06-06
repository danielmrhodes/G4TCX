#ifndef Excitation_Messenger_h
#define Excitation_Messenger_h 1

#include "Excitation.hh"
#include "G4UImessenger.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithADouble.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIcmdWithABool.hh"
#include "G4UIcmdWithoutParameter.hh"

class Excitation;
class Excitation_Messenger : public G4UImessenger {

public:

  Excitation_Messenger(Excitation* exc, G4bool prj);
  ~Excitation_Messenger();
  
private:

  Excitation* excitation;
  const G4bool proj;
  
  void SetNewValue(G4UIcommand* command, G4String newValue);

  G4UIdirectory* excitation_dir; //Excitation directory
  
  G4UIcmdWithAString* pLS_cmd; //Level scheme
  G4UIcmdWithAString* pPF_cmd; //Probabilities
  G4UIcmdWithAString* pTF_cmd; //Statistical tensor

  G4UIcmdWithAnInteger* pSel_cmd; //Selected state to populate every event
  G4UIcmdWithAnInteger* pCon_cmd; //Only consider this excited state
  G4UIcmdWithADouble* pGSS_cmd; //Ground state spin
  G4UIcmdWithoutParameter* sCon_cmd; //Simple considered state (no feeding)
  
};

#endif

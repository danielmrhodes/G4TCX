#ifndef Gamma_Source_h
#define Gamma_Source_h 1

#include "G4ParticleDefinition.hh"
#include "Gamma_Source_Messenger.hh"

class Gamma_Source_Messenger;
class Gamma_Source {

public:
   
  Gamma_Source();
  ~Gamma_Source();

  void BuildLevelScheme();
  
  G4int ChooseState();
  G4double GetEnergy() {return source_energy;}
  
  G4ParticleDefinition* GetDefinition(G4int index) {return levels[index];}
     
  void SetEnergy(G4double En) {source_energy = En;}
  void SetFileName(G4String name) {file_name = name;}
  void SetGroundStateSpin(G4double spin) {GSS = spin;} 

private:

  Gamma_Source_Messenger* messenger;

  G4double source_energy;
  G4double GSS; //ground state spin
  G4String file_name;

  G4int threadID;
  
  std::vector<G4ParticleDefinition*> levels;
  std::vector<G4double> probs;
  
};

#endif

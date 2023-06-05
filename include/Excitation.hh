#ifndef Excitation_h
#define Excitation_h 1

#include <gsl/gsl_spline2d.h>

#include "G4ParticleDefinition.hh"
#include "Polarization.hh"
#include "Excitation_Messenger.hh"

class Excitation_Messenger;
class Excitation {

public:
   
  Excitation(G4bool proj);
  ~Excitation();

  void BuildLevelScheme();
  void BuildProbabilities();
  void BuildStatisticalTensors();

  G4int ChooseState(G4double beamEn, G4double thetaCM);
  G4double GetExcitation(G4int index);

  std::vector< std::vector<G4complex> >& GetPolarization(G4int index, G4double en, G4double th, G4double ph); 

  G4ParticleDefinition* GetDefinition(G4int index) {return levels[index];}
  std::vector<G4ParticleDefinition*> GetLevels() {return levels;}
  G4double GetSpin(G4int index) {return spins[index];}

  void SetLSFile(G4String name) {lfn = name;}
  void SetPrbFile(G4String name) {pfn = name;}

  void FixState(G4int index) {selected = index;}
  void OnlyConsiderState(G4int index) {considered = index;}
  void SetGSS(G4double sp) {gss = sp;}
  void SimpleConsidered() {simple_considered = true;}
  
private:

  void ReadLevelSchemeFile(G4int Z, G4int A);
  void ReadProbFile();
  void Renormalize();
  void RenormalizeSimple();
  G4bool CanFeedConsidered(G4int index);
  
  Excitation_Messenger* messenger;
  Polarization* polar;
  const G4bool proj;

  G4int threadID;
  
  G4String lfn; //Level scheme file name
  G4String pfn; //Probabilities file name
  
  //Energy-theta grid points
  std::vector<G4double> energies;
  std::vector<G4double> thetas;
  std::vector<G4double> probs;
  std::vector<gsl_spline2d*> interps;
  
  std::vector<G4ParticleDefinition*> levels;
  std::vector<G4double> spins;
  G4int selected;
  G4int considered;
  G4double gss;
  G4bool simple_considered;

  //Stuff for 2D interpolation
  gsl_interp_accel* xacc;
  gsl_interp_accel* yacc;
  
};

#endif

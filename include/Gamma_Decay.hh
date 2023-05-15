#ifndef Gamma_Decay_h
#define Gamma_Decay_h 1

#include "Polarized_Particle.hh"
#include "Polarization_Transition.hh"

#include "G4VDecayChannel.hh"
//#include "G4PolarizationTransition.hh"
//#include "G4NuclearPolarization.hh"

class Gamma_Decay : public G4VDecayChannel {

public:

  Gamma_Decay(Polarized_Particle* Parent, Polarized_Particle* daughter, G4double BR, G4int L0,
	      G4int Lp, G4double del, G4double cc, G4bool emit);
  ~Gamma_Decay();

  G4DecayProducts* DecayIt(G4double);

  Polarized_Particle* GetDaughter() {return pDaughter;}
  G4double GetDaughterExcitation() {return pDaughter->GetExcitationEnergy();}
  //G4double GetDaughterExcitation() {return pDaughter->GetNuclearPolarization()->GetExcitationEnergy();}

  static void SetPolarization(std::vector< std::vector<G4complex> > polar);
  static G4double Pmx(G4double e, G4double p1, G4double p2);
  
private:

  Gamma_Decay(G4ParticleDefinition* Parent, G4ParticleDefinition* daughter, G4double BR);
  
  G4double parentmass;
  const G4double* theDaughterMasses;

  //G4PolarizationTransition* trans;
  Polarization_Transition* trans;

  Polarized_Particle* pParent;
  Polarized_Particle* pDaughter;

  static G4ThreadLocal std::vector< std::vector<G4complex> > polarization;
  
  G4int transL;
  G4int transLp;
  G4double delta;

  G4double convCoef;

  G4bool emit_gamma;
  
};

#endif

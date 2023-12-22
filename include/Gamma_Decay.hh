#ifndef Gamma_Decay_h
#define Gamma_Decay_h 1

#include "Polarization_Transition.hh"

#include "G4ParticleDefinition.hh"
#include "G4VDecayChannel.hh"

class Gamma_Decay : public G4VDecayChannel {

public:

  Gamma_Decay(G4ParticleDefinition* Parent, G4ParticleDefinition* daughter, G4double BR, G4double Ei,
	      G4double Ef, G4int twoJP, G4int twoJD, G4int L0, G4int Lp, G4double del, G4double cc,
	      G4bool emit, G4bool prj);
  ~Gamma_Decay();

  G4DecayProducts* DecayIt(G4double);
  G4double GetDaughterExcitation() {return EnF;}

  static std::vector<std::vector<G4complex>> GetProjectilePolarization() {return polarizationP;}
  static void SetProjectilePolarization(const std::vector<std::vector<G4complex>> polar);
  static void UnpolarizeProjectile();

  static std::vector<std::vector<G4complex>> GetRecoilPolarization() {return polarizationR;}
  static void SetRecoilPolarization(const std::vector<std::vector<G4complex>> polar);
  static void UnpolarizeRecoil();
  
  static G4double Pmx(G4double e, G4double p1, G4double p2);
  
private:

  Gamma_Decay(G4ParticleDefinition* Parent, G4ParticleDefinition* daughter, G4double BR);
  
  static void CleanProjectile();
  static void CleanRecoil();
  
  G4double parentmass;
  const G4double* theDaughterMasses;
  
  Polarization_Transition* trans;
  
  static inline G4ThreadLocal std::vector<std::vector<G4complex>> polarizationP;
  static inline G4ThreadLocal std::vector<std::vector<G4complex>> polarizationR;

  G4bool proj;
  
  G4int twoJi;
  G4int twoJf;
  G4int transL;
  G4int transLp;
  G4double delta;

  G4double EnI;
  G4double EnF;
  G4double convCoef;

  G4bool emit_gamma;
  
};

#endif

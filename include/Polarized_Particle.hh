#ifndef Polarized_Particle_h
#define Polarized_Particle_h 1

#include "G4ParticleDefinition.hh"
//#include "G4NuclearPolarization.hh"

class Polarized_Particle {

public:

  Polarized_Particle(G4ParticleDefinition* def, G4int Z, G4int A, G4double J, G4double ex);
  Polarized_Particle(G4ParticleDefinition* def, G4double J, G4double ex);
    
  ~Polarized_Particle() {;}

  G4ParticleDefinition* GetDefinition() const {return part;}
  //G4NuclearPolarization* GetNuclearPolarization() const {return polar;}
  G4double GetSpin() const {return spin;}
  G4int TwoJ() const {return G4int(2*spin + 0.01);}
  G4double GetExcitationEnergy() const {return energy;}

  std::vector< std::vector<G4complex> > GetPolarization() const {return polarization;}
  void SetPolarization(std::vector< std::vector<G4complex> > polar);
  void Unpolarize();
  
  //std::vector<std::vector<G4complex>>& GetPolarization() const {return polar->GetPolarization();}
  //void SetPolarization(std::vector< std::vector<G4complex> >& p) {polar->SetPolarization(p);}
  //void Unpolarize() {polar->Unpolarize();}
  
private:

  void Clean();

  G4ParticleDefinition* part;
  G4double spin;
  G4double energy;
  std::vector< std::vector<G4complex> > polarization;
  
  //G4NuclearPolarization* polar;

};

#endif

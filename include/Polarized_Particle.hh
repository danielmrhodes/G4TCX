#ifndef Polarized_Particle_h
#define Polarized_Particle_h 1

#include "G4ParticleDefinition.hh"
#include "G4NuclearPolarization.hh"

class Polarized_Particle {

public:

  Polarized_Particle(G4ParticleDefinition* def, G4int Z, G4int A, G4double J, G4double ex) : part(def),
											     spin(J) {
    polar = new G4NuclearPolarization(Z,A,ex);
  }
    
  ~Polarized_Particle() {}

  G4ParticleDefinition* GetDefinition() const {return part;}
  G4NuclearPolarization* GetNuclearPolarization() const {return polar;}
  G4double GetSpin() const {return spin;}
  G4int TwoJ() const {return G4int(2*spin + 0.01);}
  
  std::vector<std::vector<G4complex>>& GetPolarization() const {return polar->GetPolarization();}
  
  void SetPolarization(std::vector< std::vector<G4complex> >& p) {polar->SetPolarization(p);}
  void Unpolarize() {polar->Unpolarize();}
  
private:

  G4ParticleDefinition* part;
  G4double spin;
  G4NuclearPolarization* polar;

};

#endif

#include "Polarized_Particle.hh"

Polarized_Particle::Polarized_Particle(G4ParticleDefinition* def, G4int Z, G4int A, G4double J,
				       G4double ex) : part(def), spin(J), energy(ex) {
  
  Unpolarize();
}

Polarized_Particle::Polarized_Particle(G4ParticleDefinition* def, G4double J, G4double ex) :
  part(def), spin(J), energy(ex) {
  Unpolarize();
}

void Polarized_Particle::SetPolarization(std::vector< std::vector<G4complex> > polar) {

  Clean(); 
  for(std::vector<G4complex> pol : polar)
    polarization.push_back(pol);

  return;
}

void Polarized_Particle::Unpolarize() {

  Clean(); 
  polarization.resize(1);
  polarization[0].push_back(1.0);
  
  return;
}

void Polarized_Particle::Clean() {

  if(!polarization.empty()) {
    for(std::vector<G4complex> pol : polarization)
      pol.clear();
    polarization.clear();
  }
  
  return;
}

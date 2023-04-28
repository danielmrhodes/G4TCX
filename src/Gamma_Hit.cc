#include "Gamma_Hit.hh"

G4Allocator<Gamma_Hit>* Gamma_Hit_Allocator = 0;

void Gamma_Hit::SetDetSeg(G4int id) {

  seg = id%10;
  det = (id-seg)/10;
  
  return;
  
}

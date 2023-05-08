#include "Suppressor_Hit.hh"

G4ThreadLocal G4Allocator<Suppressor_Hit>* Suppressor_Hit_Allocator = 0;

void Suppressor_Hit::SetDetSeg(G4int id) {

  seg = id%10;
  det = (id-seg)/10;
  
  return;
  
}

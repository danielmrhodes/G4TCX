#include "Ion_Hit.hh"

G4ThreadLocal G4Allocator<Ion_Hit>* Ion_Hit_Allocator = 0;

Ion_Hit::Ion_Hit() : proj(false),rec(false) {}

Ion_Hit::Ion_Hit(const Ion_Hit* other) : edep(other->GetEdep()),pos(other->GetPos()),
					 det(other->GetDetector()),ring(other->GetRing()),
					 sector(other->GetSector()),proj(other->IsProjectile()),
					 rec(other->IsRecoil()) {}

Ion_Hit::~Ion_Hit() {}

void Ion_Hit::SetSeg(G4int id) {

  if(id > 10000) {
      det=1;
      id-=10000;
    }
    else {
      det=0;
    }
  
    sector = id%100;
    ring = (id-sector)/100;

    return;
  
}

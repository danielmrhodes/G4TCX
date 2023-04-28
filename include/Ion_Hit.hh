#ifndef Ion_Hit_h
#define Ion_Hit_h 1

#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"
#include "G4ThreeVector.hh"

class Ion_Hit : public G4VHit {
  
public:

  Ion_Hit();
  Ion_Hit(const Ion_Hit* other);
  ~Ion_Hit();

  inline void* operator new(size_t);
  inline void operator delete(void *aHit);

  inline void SetEdep(G4double de) {edep = de;}
  inline void SetPos(G4ThreeVector vec) {pos = vec;}
  
  inline void SetIsProjectile() {proj = true;}
  inline void SetIsRecoil() {rec = true;}
  inline void SetIsNotProjectile() {proj = false;}
  inline void SetIsNotRecoil() {rec = false;}

  inline void SetDet(G4int d) {det = d;}
  inline void SetRing(G4int r) {ring = r;}
  inline void SetSector(G4int sc) {sector = sc;}
  
  void SetSeg(G4int id);
  
  G4double GetEdep() const {return edep;}
  G4ThreeVector GetPos() const {return pos;}
  
  G4int GetRing() const {return ring;}
  G4int GetSector() const {return sector;}
  G4int GetDetector() const {return det;}

  G4bool IsRing() {return (bool)ring;}
  G4bool IsSector() {return (bool)sector;}

  G4bool IsProjectile() const {return proj;}
  G4bool IsRecoil() const {return rec;}
  
private:
  
  G4double edep;
  G4ThreeVector pos;

  G4int det, ring, sector;

  G4bool proj;
  G4bool rec;
  
};

typedef G4THitsCollection<Ion_Hit> Ion_Hit_Collection;

extern G4Allocator<Ion_Hit>* Ion_Hit_Allocator;

inline void* Ion_Hit::operator new(size_t) {
  
  if(!Ion_Hit_Allocator) {
    Ion_Hit_Allocator = new G4Allocator<Ion_Hit>;
  }
  
  return (void*)Ion_Hit_Allocator->MallocSingle();
}

inline void Ion_Hit::operator delete(void* aHit) {
  Ion_Hit_Allocator->FreeSingle((Ion_Hit*)aHit);
}

#endif

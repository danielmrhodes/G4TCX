#ifndef Suppressor_Hit_h
#define Suppressor_Hit_h 1

#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"
#include "G4ThreeVector.hh"

class Suppressor_Hit : public G4VHit {
  
public:

  Suppressor_Hit() {;}
  ~Suppressor_Hit() {;}

  inline void* operator new(size_t);
  inline void operator delete(void *aHit);

  //inline void SetEdep(G4double de) {edep = de;}
  //inline void SetPos(G4ThreeVector vec) {pos = vec;}

  void SetDetSeg(G4int id);

  //G4double GetEdep() const {return edep;}
  //G4ThreeVector GetPos() const {return pos;}
 
  G4int GetSegment() const {return seg;}
  G4int GetDetector() const {return det;}
  
private:
  
  //G4double edep;
  // G4ThreeVector pos;
  G4int det, seg;

  
};

typedef G4THitsCollection<Suppressor_Hit> Suppressor_Hit_Collection;

extern G4ThreadLocal G4Allocator<Suppressor_Hit>* Suppressor_Hit_Allocator;

inline void* Suppressor_Hit::operator new(size_t) {
  
  if(!Suppressor_Hit_Allocator)
    Suppressor_Hit_Allocator = new G4Allocator<Suppressor_Hit>;
  
  return (void*)Suppressor_Hit_Allocator->MallocSingle();
}

inline void Suppressor_Hit::operator delete(void* aHit) {
  Suppressor_Hit_Allocator->FreeSingle((Suppressor_Hit*)aHit);
}

#endif

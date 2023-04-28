#ifndef SuppressorSD_h
#define SuppressorSD_h 1

#include "G4VSensitiveDetector.hh"
#include "Suppressor_Hit.hh"

class SuppressorSD : public G4VSensitiveDetector {
  
public:

  SuppressorSD(G4String name);
  ~SuppressorSD();

  G4bool ProcessHits(G4Step* step, G4TouchableHistory*);
  void Initialize(G4HCofThisEvent*);
  void EndOfEvent(G4HCofThisEvent*);

private:
  
  Suppressor_Hit_Collection* HC;
  
};

#endif

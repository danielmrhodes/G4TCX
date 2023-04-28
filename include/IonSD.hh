#ifndef IonSD_h
#define IonSD_h 1

#include "G4VSensitiveDetector.hh"
#include "Ion_Hit.hh"

class IonSD : public G4VSensitiveDetector {
  
public:

  IonSD(G4String name);
  ~IonSD();

  G4bool ProcessHits(G4Step* step, G4TouchableHistory*);
  void Initialize(G4HCofThisEvent*);
  void EndOfEvent(G4HCofThisEvent*);

  void SetProjectileName(G4String name) {proj_name = name;}
  void SetRecoilName(G4String name) {recoil_name = name;}
  
private:

  G4String proj_name;
  G4String recoil_name;

  void ConsolidateHits();
  void CombineRings();
  
  Ion_Hit_Collection* HC;

};

#endif

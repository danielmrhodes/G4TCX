#ifndef GammaSD_h
#define GammaSD_h 1

#include "G4VSensitiveDetector.hh"
#include "Gamma_Hit.hh"
#include "Tracking_Action.hh"

class GammaSD : public G4VSensitiveDetector {
  
public:

  GammaSD(G4String name);
  ~GammaSD();

  G4bool ProcessHits(G4Step* step, G4TouchableHistory*);
  void Initialize(G4HCofThisEvent*);
  void EndOfEvent(G4HCofThisEvent*);

  void SetTrackingAction(Tracking_Action* tr) {trkAct = tr;}
  
private:
  
  Gamma_Hit_Collection* HC;
  Tracking_Action* trkAct;

  //Map a detector to a list of track ids
  std::map<G4int,std::vector<G4int>> detMap;
  
};

#endif

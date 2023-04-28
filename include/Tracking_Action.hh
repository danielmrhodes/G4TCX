#ifndef Tracking_Action_h
#define Tracking_Action_h 1

#include "G4UserTrackingAction.hh"
#include "Primary_Generator.hh"

#include <map>

class Tracking_Action : public G4UserTrackingAction {

public:

  Tracking_Action();
  ~Tracking_Action();

  void PreUserTrackingAction(const G4Track* track);

  void Clear();
  void SetMode(Primary_Generator::MODE md) {mode = md;}
  void SetProjectileName(G4String name) {projName = name;}
  void SetIsSimpleSource() {simple_source = true;}

  std::vector<G4int> GetProjGammas() {return projGammas;}
  std::map<G4int,std::vector<G4int>> GetIDMap() {return idMap;}
  std::map<G4int,G4double> GetEnergyMap() {return enMap;}
  
private:

  Primary_Generator::MODE mode;
  G4bool simple_source;
  
  //List of projectile or recoil track IDs
  std::vector<G4int> ionIDs;

  //Keep track of the projectile
  G4String projName;
  std::vector<G4int> projIDs;
  std::vector<G4int> projGammas;

  //Map a gamma id to list of secondary ids
  std::map<G4int,std::vector<G4int>> idMap;

  //Map a gamma to its lab-frame energy
  std::map<G4int,G4double> enMap;
  
};
  
#endif

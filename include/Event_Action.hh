#ifndef Event_Action_h
#define Event_Action_h 1

#include "Primary_Generator.hh"
#include "G4UserEventAction.hh"
#include "G4Event.hh"
#include "G4String.hh"

class Event_Action : public G4UserEventAction {
  
public:
   
  Event_Action() : nEvents(0), perEvent(1) {;}
  ~Event_Action() {;}

  void BeginOfEventAction(const G4Event* evt);
  
  void SetNEvents(G4int n) {nEvents = n;}
  void SetPerEvent(G4int n);
  
private:

  G4int nEvents;
  G4int perEvent;
  
};

#endif

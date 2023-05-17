#include "Event_Action.hh"

void Event_Action::BeginOfEventAction(const G4Event* evt) {
  
  G4int id = evt->GetEventID() + 1;
  if(!((id)%perEvent))
    std::cout << "Event " << id << " (" << G4int(100 * G4double(id)/G4double(nEvents)) << "%)\r"
	   << std::flush;

  return;
}

void Event_Action::SetPerEvent(G4int num) {

  if(num < 1001)
    perEvent = 1;
  else if(num < 10001)
    perEvent = 100;
  else if(num < 100001)
    perEvent = 500;
  else if(num < 1000001)
    perEvent = 1000;
  else if(num < 10000001)
    perEvent = 2000;
  else
    perEvent = 3000;
  
  return;
}

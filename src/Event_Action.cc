#include "Event_Action.hh"
#include "G4Event.hh"

Event_Action::Event_Action() {

  messenger = new Event_Action_Messenger(this);

  nEvents = 0;
  
  fname = "output.dat";
  output = NULL;

  dname = "info.dat";
  diagnostics = NULL;

  owc = false;
  
}

Event_Action::~Event_Action() {

  delete messenger;
  
}

void Event_Action::BeginOfEventAction(const G4Event* evt) {

  G4int id = evt->GetEventID();
  if(!(id%perEvent))
    G4cout << "Event " << id << " (" << G4int(100 * G4double(id)/G4double(nEvents)) << "%)\r"
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
  else
    perEvent = 2000;
  
  return;
}

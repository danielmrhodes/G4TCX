#ifndef Event_Action_h
#define Event_Action_h 1

#include "Event_Action_Messenger.hh"
#include "Primary_Generator.hh"
#include "G4UserEventAction.hh"
#include "G4String.hh"
#include "stdio.h"

class Event_Action_Messenger;
class Event_Action : public G4UserEventAction {
  
public:
   
  Event_Action();
  ~Event_Action();

  void BeginOfEventAction(const G4Event* evt);
  void EndOfEventAction(const G4Event* evt);

  G4String GetOutputFileName() {return fname;}
  FILE* GetOutputFile() {return output;}

  G4String GetDiagnosticsFileName() {return dname;}
  FILE* GetDiagnosticsFile() {return diagnostics;}

  void SetOutputFileName(G4String n) {fname = n;}
  void SetOutputFile(FILE* f) {output = f;}

  void SetDiagnosticsFileName(G4String n) {dname = n;}
  void SetDiagnosticsFile(FILE* f) {diagnostics = f;}
  
  void SetNEvents(G4int n) {nEvents = n;}
  void SetPerEvent(G4int n);

  void SetPrimaryGenerator(Primary_Generator* gn) {gen = gn;}
  
  void OWC() {owc = true;}
  
private:

  Event_Action_Messenger* messenger;
  Primary_Generator* gen;
  
  G4int nEvents;
  G4int perEvent;
  G4bool owc;

  G4String fname;
  FILE* output;

  G4String dname;
  FILE* diagnostics;
  
};

#endif

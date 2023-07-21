#ifndef Run_Action_h
#define Run_Action_h 1

#include "Run_Action_Messenger.hh"
#include "Run.hh"

#include "G4UserRunAction.hh"
#include "G4Run.hh"

class Run_Action_Messenger;
class Run_Action : public G4UserRunAction {
  
public:

  Run_Action();
  ~Run_Action();

  void BeginOfRunAction(const G4Run*);
  void EndOfRunAction(const G4Run*);

  G4String GetOutputFileName() const {return fname;}
  G4String GetDiagnosticsFileName() const {return dname;}

  void SetGammaTrigger(G4int trig) {gammaTrigger = trig;}
  
  void SetOutputFileName(G4String n) {fname = n;}
  void SetDiagnosticsFileName(G4String n) {dname = n;}
  
  void OnlyWriteCoincidences() {owc = true;}
  void WriteDiagnostics() {write_diag = true;}
  

private:
  
  G4Run* GenerateRun() {return new Run();}
  
  Run_Action_Messenger* messenger;
  
  G4bool owc;
  G4bool write_diag;

  G4int gammaTrigger;
  
  G4String fname;
  G4String dname;

};
  
#endif
